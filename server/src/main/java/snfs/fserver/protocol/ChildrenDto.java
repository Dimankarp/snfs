package snfs.fserver.protocol;

import lombok.Data;

import java.nio.ByteBuffer;
import java.util.List;

@Data
public class ChildrenDto implements ByteSerializable{
    private List<DentryDto> children;

    public void putToBuffer(ByteBuffer buffer) {
        var len = children.size();
        buffer.putInt(len);
        children.forEach(child -> child.putToBuffer(buffer));
    }
}
