package snfs.fserver.protocol;

import java.nio.ByteBuffer;

public class ResponseBuilder {
    private static final int BUF_SZ = 4096;
    private ByteBuffer buffer;

    public ResponseBuilder() {
        buffer = ByteBuffer.allocate(BUF_SZ);
    }

    public ResponseBuilder addItem(ByteSerializable item) {
        item.putToBuffer(buffer);
        return this;
    }

    public byte[] toSizedByteArray() {
        var length = buffer.position();
        byte[] buf = new byte[length + 4];
        buffer.rewind();
        buffer.get(buf, 4, length);
        buffer.reset();
        buffer.putInt(length);
        buffer.rewind();
        buffer.get(buf, 0, 4);
        return buf;
    }

}
