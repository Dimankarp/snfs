package snfs.fserver.entity;

import jakarta.persistence.*;
import lombok.Data;
import snfs.fserver.protocol.InodeType;

import java.util.List;

@Entity
@Data
public class Inode {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long no;

    @Enumerated(EnumType.STRING)
    private InodeType type;

    //Probably should've used blobs and stream, but...
    private String text;

    @OneToMany(fetch = FetchType.LAZY)
    @JoinTable(name = "children", joinColumns = {@JoinColumn(name = "parent_no")}, inverseJoinColumns =
            {@JoinColumn(name = "child_no")})
    private List<Dentry> files;


}
