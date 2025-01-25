package snfs.fserver.protocol;

import java.nio.ByteBuffer;

public interface ByteSerializable {
    public void putToBuffer(ByteBuffer buffer);
}
