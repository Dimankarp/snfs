package snfs.fserver.protocol;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
public class ResponseBuilder {
    private static final int BUF_SZ = 4096;
    private ByteBuffer buffer;

    public ResponseBuilder() {
        buffer = ByteBuffer.allocate(BUF_SZ);
        buffer.order(ByteOrder.LITTLE_ENDIAN);
    }

    public ResponseBuilder addItem(ByteSerializable item) {
        item.putToBuffer(buffer);
        return this;
    }

    public byte[] toSizedByteArray() {
        var length = buffer.position();
        byte[] buf = new byte[length + 8];
        buffer.rewind();
        buffer.get(buf, 8, length);
        buffer.clear();
        buffer.putLong(length);
        buffer.rewind();
        buffer.get(buf, 0, 8);
        return buf;
    }

}
