package snfs.fserver.repository;

import org.springframework.data.jpa.repository.JpaRepository;
import snfs.fserver.entity.Dentry;

public interface DentryRepository extends JpaRepository<Dentry, Long> {
}
