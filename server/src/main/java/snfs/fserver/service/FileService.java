package snfs.fserver.service;

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
    protected ChildrenMsgDto childrenToDto(List<Dentry> dentries) {
        var children = ChildrenMsgDto.newBuilder();
        children.setStatus(ErrStatus.OK);
        children.addAllChildren(dentries.stream().map(this::dentryToDto).toList());
        return children.build();
    }

    @Transactional
    protected DentryDto dentryToDto(Dentry dentry) {
        var builder = DentryDto.newBuilder();
        builder.setInode(inodeToDto(dentry.getInode()));
        builder.setName(dentry.getName());
        return builder.build();
    }

    private InodeMsgDto errorInode(ErrStatus status) {
        return InodeMsgDto.newBuilder().setStatus(status).build();
    }

    private ChildrenMsgDto errorChildren(ErrStatus status) {
        return ChildrenMsgDto.newBuilder().setStatus(status).build();
    }

    private MsgDto errorMsg(ErrStatus status) {
        return MsgDto.newBuilder().setStatus(status).build();
    }

    private TextMsgDto errorText(ErrStatus status) {
        return TextMsgDto.newBuilder().setStatus(status).build();
    }

    private InodeDto inodeToDto(Inode inode) {
        var builder = InodeDto.newBuilder();
        builder.setType(inode.getType());
        builder.setNo(Math.toIntExact(inode.getNo()));
        builder.setSize(inode.getText() != null ? inode.getText().length() : 0);
        return builder.build();
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
    public InodeMsgDto mount(String token) {

        var tokenOpt = tokenRepository.findByToken(token);
        var tk = tokenOpt.orElseGet(() -> registerToken(token));
        var indodeDto = inodeToDto(tk.getRoot());
        return InodeMsgDto.newBuilder().setStatus(ErrStatus.OK).setInode(indodeDto).build();
    }

    @Transactional
    public InodeMsgDto create(String tk, Long dir, String name, InodeType type) {
        var dirOpt = inodeRepository.findById(dir);
        if (dirOpt.isEmpty()) {
            return errorInode(ErrStatus.MISSING);
        }
        var dirNode = dirOpt.get();
        if (dirNode.getType() != InodeType.DIR) {
            return errorInode(ErrStatus.NOTDIR);
        }
        for (var entry : dirNode.getFiles()) {
            if (entry.getName().equals(name)) {
                return errorInode(ErrStatus.DUPLICATE);
            }
        }
        var file = createFile(name, type);
        dentryRepository.save(file);
        dirNode.getFiles().add(file);
        return InodeMsgDto.newBuilder().setStatus(ErrStatus.OK).setInode(inodeToDto(file.getInode())).build();
    }

    @Transactional
    public ChildrenMsgDto children(String tk, Long dir) {
        var dirOpt = inodeRepository.findById(dir);
        if (dirOpt.isEmpty()) {
            return errorChildren(ErrStatus.MISSING);
        }
        var dirNode = dirOpt.get();
        if (dirNode.getType() != InodeType.DIR) {
            return errorChildren(ErrStatus.NOTDIR);
        }
        return childrenToDto(dirNode.getFiles());
    }

    @Transactional
    public MsgDto remove(String tk, Long dir, String name) {
        var dirOpt = inodeRepository.findById(dir);
        if (dirOpt.isEmpty()) {
            return errorMsg(ErrStatus.MISSING);
        }
        var dirNode = dirOpt.get();
        if (dirNode.getType() != InodeType.DIR) {
            return errorMsg(ErrStatus.NOTDIR);
        }
        for (var child : dirNode.getFiles()) {
            if (child.getName().equals(name)) {
                dentryRepository.delete(child);
                dirNode.getFiles().remove(child);
                return errorMsg(ErrStatus.OK);
            }
        }
        return errorMsg(ErrStatus.MISSING);
    }

    @Transactional
    public TextMsgDto read(String tk, Long ino, Long offset) {
        var fileOpt = inodeRepository.findById(ino);
        if (fileOpt.isEmpty()) {
            return errorText(ErrStatus.MISSING);
        }
        var fileNode = fileOpt.get();
        if (fileNode.getType() != InodeType.REG) {
            return errorText(ErrStatus.ISDIR);
        }
        var text = fileNode.getText();
        if (text == null || offset >= text.length()) {
            return errorText(ErrStatus.EMPTY);
        }
        return TextMsgDto.newBuilder().setStatus(ErrStatus.OK).setText(text.substring(Math.toIntExact(offset))).build();
    }

    @Transactional
    public MsgDto write(String tk, Long ino, String text, Long offset) {
        var fileOpt = inodeRepository.findById(ino);
        if (fileOpt.isEmpty()) {
            return errorMsg(ErrStatus.MISSING);
        }
        var fileNode = fileOpt.get();
        if (fileNode.getType() != InodeType.REG) {
            return errorMsg(ErrStatus.ISDIR);
        }
        var oldText = fileNode.getText();
        var sb = new StringBuilder();
        if (oldText != null)
            sb.append(oldText, 0, Math.min(Math.toIntExact(offset), oldText.length()));
        var spaces = offset - sb.length();
        sb.append(" ".repeat((int) spaces));
        sb.append(text);
        fileNode.setText(sb.toString());
        return errorMsg(ErrStatus.OK);
    }

}
