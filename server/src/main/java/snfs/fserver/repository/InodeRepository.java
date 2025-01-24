package snfs.fserver.repository;

import org.springframework.data.jpa.repository.JpaRepository;
import snfs.fserver.entity.Inode;

public interface InodeRepository extends JpaRepository<Inode, Long> {
}
