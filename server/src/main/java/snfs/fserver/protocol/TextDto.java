package snfs.fserver.protocol;

import lombok.Data;

import java.nio.ByteBuffer;

@Data
public class TextDto implements ByteSerializable {
    private String text;

    public void putToBuffer(ByteBuffer buffer) {
        buffer.putInt(text.getBytes().length);
        buffer.put(text.getBytes());
    }
}
