package snfs.fserver.protocol;

import lombok.Data;

import java.nio.ByteBuffer;

@Data
public class InodeDto implements ByteSerializable {
    private int no;
    private InodeType type;
    private int size;

    public void putToBuffer(ByteBuffer buffer) {
        buffer.putInt(no);
        buffer.putInt(type.ordinal());
        buffer.putInt(size);
    }
}
