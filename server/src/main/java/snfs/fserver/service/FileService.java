package snfs.fserver.service;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import snfs.fserver.entity.Dentry;
import snfs.fserver.entity.Inode;
import snfs.fserver.entity.Token;
import snfs.fserver.protocol.*;
import snfs.fserver.repository.DentryRepository;
import snfs.fserver.repository.InodeRepository;
import snfs.fserver.repository.TokenRepository;

import java.util.List;

@Service
public class FileService {


    private final Logger logger = LoggerFactory.getLogger(FileService.class);
    private final TokenRepository tokenRepository;
    private final InodeRepository inodeRepository;
    private final DentryRepository dentryRepository;

    public FileService(TokenRepository tokenRepository, InodeRepository inodeRepository, DentryRepository dentryRepository) {
        this.tokenRepository = tokenRepository;
        this.inodeRepository = inodeRepository;
        this.dentryRepository = dentryRepository;
    }

    private Token registerToken(String token) {
        var newToken = new Token();
        newToken.setToken(token);
        var root = new Inode();
        root.setType(InodeType.DIR);
        newToken.setRoot(root);
        return tokenRepository.save(newToken);
    }

    @Transactional
    protected Token getToken(String tk) {
        var tokenOpt = tokenRepository.findByToken(tk);
        return tokenOpt.orElseGet(() -> registerToken(tk));
    }

    @Transactional
    protected ChildrenDto childrenToDto(List<Dentry> dentries) {
        var dto = new ChildrenDto();
        dto.setChildren(dentries.stream().map(this::dentryToDto).toList());
        return dto;
    }

    @Transactional
    protected DentryDto dentryToDto(Dentry dentry) {
        var dto = new DentryDto();
        dto.setName(dentry.getName());
        dto.setInode(inodeToDto(dentry.getInode()));
        return dto;
    }

    private MsgDto msgDto(ErrStatus status) {
        var dto = new MsgDto();
        dto.setStatus(status);
        return dto;
    }

    private InodeDto inodeToDto(Inode inode) {
        var dto = new InodeDto();
        dto.setNo(Math.toIntExact(inode.getNo()));
        dto.setType(inode.getType());
        dto.setSize(inode.getText() == null ? 0 : inode.getText().length());
        return dto;
    }

    private Dentry createFile(String name, InodeType type) {
        var inode = new Inode();
        inode.setType(type);
        var dentry = new Dentry();
        dentry.setName(name);
        dentry.setInode(inode);
        return dentry;
    }

    @Transactional
    public ResponseBuilder mount(String token) {

        var tokenOpt = tokenRepository.findByToken(token);
        var tk = tokenOpt.orElseGet(() -> registerToken(token));
        var inodeDto = inodeToDto(tk.getRoot());
        var builder = new ResponseBuilder();
        return builder.addItem(msgDto(ErrStatus.OK)).addItem(inodeDto);
    }

    @Transactional
    public ResponseBuilder create(String tk, Long dir, String name, InodeType type) {
        var dirOpt = inodeRepository.findById(dir);
        var builder = new ResponseBuilder();
        if (dirOpt.isEmpty()) {
            return builder.addItem(msgDto(ErrStatus.MISSING));
        }
        var dirNode = dirOpt.get();
        if (dirNode.getType() != InodeType.DIR) {
            return builder.addItem(msgDto(ErrStatus.NOTDIR));
        }
        for (var entry : dirNode.getFiles()) {
            if (entry.getName().equals(name)) {
                return builder.addItem(msgDto(ErrStatus.DUPLICATE));
            }
        }
        var file = createFile(name, type);
        dentryRepository.save(file);
        dirNode.getFiles().add(file);
        logger.debug("Created file with name {}", name);
        return builder.addItem(msgDto(ErrStatus.OK)).addItem(inodeToDto(file.getInode()));
    }

    @Transactional
    public ResponseBuilder children(String tk, Long dir) {
        var builder = new ResponseBuilder();
        var dirOpt = inodeRepository.findById(dir);
        if (dirOpt.isEmpty()) {
            return builder.addItem(msgDto(ErrStatus.MISSING));
        }
        var dirNode = dirOpt.get();
        if (dirNode.getType() != InodeType.DIR) {
            return builder.addItem(msgDto(ErrStatus.NOTDIR));
        }

        for (var entry : dirNode.getFiles()) {
            logger.debug("Dentry: {} {}", entry.getInode().getNo(), entry.getName());
        }
        return builder.addItem(msgDto(ErrStatus.OK)).addItem(childrenToDto(dirNode.getFiles()));
    }

    @Transactional
    public ResponseBuilder remove(String tk, Long dir, String name) {
        var dirOpt = inodeRepository.findById(dir);
        var builder = new ResponseBuilder();
        if (dirOpt.isEmpty()) {
            return builder.addItem(msgDto(ErrStatus.MISSING));
        }
        var dirNode = dirOpt.get();
        if (dirNode.getType() != InodeType.DIR) {
            return builder.addItem(msgDto(ErrStatus.NOTDIR));
        }
        for (var child : dirNode.getFiles()) {
            if (child.getName().equals(name)) {
                dentryRepository.delete(child);
                dirNode.getFiles().remove(child);
                return builder.addItem(msgDto(ErrStatus.OK));
            }
        }
        return builder.addItem(msgDto(ErrStatus.MISSING));
    }

    @Transactional
    public ResponseBuilder lookup(String tk, Long dir, String name) {
        var dirOpt = inodeRepository.findById(dir);
        var builder = new ResponseBuilder();
        if (dirOpt.isEmpty()) {
            return builder.addItem(msgDto(ErrStatus.MISSING));
        }
        var dirNode = dirOpt.get();
        if (dirNode.getType() != InodeType.DIR) {
            return builder.addItem(msgDto(ErrStatus.NOTDIR));
        }
        for (var child : dirNode.getFiles()) {
            if (child.getName().equals(name)) {
                return builder.addItem(msgDto(ErrStatus.OK)).addItem(inodeToDto(child.getInode()));
            }
        }
        return builder.addItem(msgDto(ErrStatus.MISSING));
    }

    @Transactional
    public ResponseBuilder read(String tk, Long ino, Long offset) {
        var fileOpt = inodeRepository.findById(ino);
        var builder = new ResponseBuilder();
        if (fileOpt.isEmpty()) {
            return builder.addItem(msgDto(ErrStatus.MISSING));
        }
        var fileNode = fileOpt.get();
        if (fileNode.getType() != InodeType.REG) {
            return builder.addItem(msgDto(ErrStatus.ISDIR));
        }
        var text = fileNode.getText();
        if (text == null || offset >= text.length()) {
            return builder.addItem(msgDto(ErrStatus.EMPTY));
        }
        var dto = new TextDto();
        dto.setText(text);
        return builder.addItem(msgDto(ErrStatus.OK)).addItem(dto);
    }

    @Transactional
    public ResponseBuilder write(String tk, Long ino, String text, Long offset) {
        var fileOpt = inodeRepository.findById(ino);
        var builder = new ResponseBuilder();
        if (fileOpt.isEmpty()) {
            return builder.addItem(msgDto(ErrStatus.MISSING));
        }
        var fileNode = fileOpt.get();
        if (fileNode.getType() != InodeType.REG) {
            return builder.addItem(msgDto(ErrStatus.ISDIR));
        }
        var oldText = fileNode.getText();
        var sb = new StringBuilder();
        if (oldText != null) sb.append(oldText, 0, Math.min(Math.toIntExact(offset), oldText.length()));
        var spaces = offset - sb.length();
        sb.append(" ".repeat((int) spaces));
        sb.append(text);
        fileNode.setText(sb.toString());
        return builder.addItem(msgDto(ErrStatus.OK));
    }

}
