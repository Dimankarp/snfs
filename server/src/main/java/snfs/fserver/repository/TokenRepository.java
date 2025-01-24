package snfs.fserver.repository;

import org.springframework.data.jpa.repository.JpaRepository;
import snfs.fserver.entity.Token;

import java.util.Optional;

public interface TokenRepository extends JpaRepository<Token, Long> {

    Optional<Token> findByToken(String token);
}
