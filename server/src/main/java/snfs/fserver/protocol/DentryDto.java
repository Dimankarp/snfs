package snfs.fserver.protocol;

import lombok.Data;

import java.nio.ByteBuffer;

@Data
public class DentryDto implements ByteSerializable {
    private static final int NAME_SZ = 128;
    private String name;
    private InodeDto inode;

    public void putToBuffer(ByteBuffer buffer) {
        var len = name.getBytes().length;
        buffer.put(name.getBytes());
        if (len < NAME_SZ) {
            var zeroes = new byte[NAME_SZ - len];
            buffer.put(zeroes);
        }
        inode.putToBuffer(buffer);
    }
}
