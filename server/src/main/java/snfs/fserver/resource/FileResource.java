package snfs.fserver.resource;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import snfs.fserver.protocol.InodeType;
import snfs.fserver.service.FileService;

import java.io.IOException;

@RestController("/api")
public class FileResource {

    private final Logger logger = LoggerFactory.getLogger(FileResource.class);
    private final FileService fileService;

    public FileResource(FileService fileService) {
        this.fileService = fileService;
    }

    /* Returns InodeMsg with root */
    @GetMapping("/mount")
    public ResponseEntity<byte[]> mount(@RequestParam String token) {
        var res = fileService.mount(token);
        return ResponseEntity.ok(res.toSizedByteArray());
    }

    /* Returns InodeMsg with entry */
    @GetMapping("/create")
    public ResponseEntity<byte[]> create(@RequestParam String token, @RequestParam Long dir,
                                         @RequestParam String name, @RequestParam InodeType type) {
        var res = fileService.create(token, dir, name, type);
        return ResponseEntity.ok(res.toSizedByteArray());
    }

    /* Returns InodeMsg with entry */
    @GetMapping("/lookup")
    public ResponseEntity<byte[]> lookup(@RequestParam String token, @RequestParam Long dir,
                                         @RequestParam String name) {
        var res = fileService.lookup(token, dir, name);
        return ResponseEntity.ok(res.toSizedByteArray());
    }

    /* Returns Msg */
    @GetMapping("/remove")
    public ResponseEntity<byte[]> remove(@RequestParam String token, @RequestParam Long dir,
                                         @RequestParam String name) {
        var res = fileService.remove(token, dir, name);
        return ResponseEntity.ok(res.toSizedByteArray());
    }

    /* Returns ChildrenMsg with entries */
    @GetMapping("/children")
    public ResponseEntity<byte[]> children(@RequestParam String token, @RequestParam Long dir) {
        var res = fileService.children(token, dir);
        return ResponseEntity.ok(res.toSizedByteArray());
    }

    /* Returns TextMsg */
    @GetMapping("/read")
    public ResponseEntity<byte[]> read(@RequestParam String token, @RequestParam Long ino, @RequestParam Long offset) {
        var res = fileService.read(token, ino, offset);
        logger.info("Read: {}", res);
        return ResponseEntity.ok(res.toSizedByteArray());
    }

    /* Returns Msg */
    @GetMapping("/write")
    public ResponseEntity<byte[]> write(@RequestParam String token, @RequestParam Long ino,
                                        @RequestParam String text, @RequestParam Long offset) {
        var res = fileService.write(token, ino, text, offset);
        logger.info("Wrote: {}", res);
        return ResponseEntity.ok(res.toSizedByteArray());
    }


}
