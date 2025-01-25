package snfs.fserver.protocol;

import lombok.Data;

import java.nio.ByteBuffer;

@Data
public class MsgDto implements ByteSerializable {
    private ErrStatus status;

    public void putToBuffer(ByteBuffer buffer) {
        buffer.putInt(status.ordinal());
    }
}
