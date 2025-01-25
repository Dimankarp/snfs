package snfs.fserver.resource;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import snfs.fserver.protocol.InodeType;
import snfs.fserver.service.FileService;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

@RestController("/api")
public class FileResource {

    private final Logger logger = LoggerFactory.getLogger(FileResource.class);
    private final FileService fileService;

    public FileResource(FileService fileService) {
        this.fileService = fileService;
    }

    /* Returns InodeMsg with root */
    @GetMapping("/mount")
    public ResponseEntity<byte[]> mount(@RequestParam String token) throws IOException {
        ByteBuffer  bs = ByteBuffer.allocate(4096).order(ByteOrder.LITTLE_ENDIAN).putLong(16).putInt(-10);
        byte[] buf = new byte[bs.position()];
        bs.rewind();
        var res = bs.get(buf);
                //fileService.mount(token);
        logger.info("Mounted file: {}", buf);

        return ResponseEntity.ok(buf);
    }

    /* Returns InodeMsg with entry */
    @GetMapping("/create")
    public ResponseEntity<byte[]> create(@RequestParam String token, @RequestParam Long dir,
                                         @RequestParam String name, @RequestParam InodeType type) {
        var res = fileService.create(token, dir, name, type);
        logger.info("CReated file: {}", res);
        return ResponseEntity.ok(res.toByteArray());
    }

    /* Returns Msg */
    @GetMapping("/remove")
    public ResponseEntity<byte[]> remove(@RequestParam String token, @RequestParam Long dir,
                                         @RequestParam String name) {
        var res = fileService.remove(token, dir, name);
        logger.info("Removed file: {}", res);
        return ResponseEntity.ok(res.toByteArray());
    }

    /* Returns ChildrenMsg with entries */
    @GetMapping("/children")
    public ResponseEntity<byte[]> children(@RequestParam String token, @RequestParam Long dir) {
        var res = fileService.children(token, dir);
        logger.info("Got Children file: {}", res);
        return ResponseEntity.ok(res.toByteArray());
    }

    /* Returns TextMsg */
    @GetMapping("/read")
    public ResponseEntity<byte[]> read(@RequestParam String token, @RequestParam Long ino, @RequestParam Long offset) {
        var res = fileService.read(token, ino, offset);
        logger.info("Read: {}", res);
        return ResponseEntity.ok(res.toByteArray());
    }

    /* Returns Msg */
    @GetMapping("/write")
    public ResponseEntity<byte[]> write(@RequestParam String token, @RequestParam Long ino,
                                        @RequestParam String text, @RequestParam Long offset) {
        var res = fileService.write(token, ino, text, offset);
        logger.info("Wrote: {}", res);
        return ResponseEntity.ok(res.toByteArray());
    }


}
