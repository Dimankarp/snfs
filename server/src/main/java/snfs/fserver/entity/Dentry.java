package snfs.fserver.entity;

import jakarta.persistence.*;
import lombok.Data;

@Entity
@Data
public class Dentry {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    private String name;

    @OneToOne(fetch = FetchType.EAGER, cascade = CascadeType.PERSIST)
    @JoinColumn(name = "inode_no", referencedColumnName = "no")
    private Inode inode;

}
