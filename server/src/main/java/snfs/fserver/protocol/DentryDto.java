package snfs.fserver.protocol;

import lombok.Data;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

@Data
public class DentryDto implements ByteSerializable {
    private static final int NAME_SZ = 128;
    private String name;
    private InodeDto inode;

    public void putToBuffer(ByteBuffer buffer) {
        var bytes = name.getBytes(StandardCharsets.US_ASCII);
        buffer.put(bytes);
        if (bytes.length < NAME_SZ) {
            var zeroes = new byte[NAME_SZ - bytes.length];
            buffer.put(zeroes);
        }
        inode.putToBuffer(buffer);
    }
}
