package snfs.fserver.protocol;

import lombok.Data;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

@Data
public class TextDto implements ByteSerializable {
    private String text;

    public void putToBuffer(ByteBuffer buffer) {
        var bytes = text.getBytes(StandardCharsets.US_ASCII);
        buffer.putInt(bytes.length);
        buffer.put(bytes);
    }
}
