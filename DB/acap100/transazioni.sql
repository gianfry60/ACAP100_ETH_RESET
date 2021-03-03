-- --------------------------------------------------------
-- Host:                         62.173.173.47
-- Versione server:              8.0.23 - MySQL Community Server - GPL
-- S.O. server:                  Win64
-- HeidiSQL Versione:            11.2.0.6213
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

-- Dump della struttura di tabella acap100.transazioni
CREATE TABLE IF NOT EXISTS `transazioni` (
  `IdTransazioni` int NOT NULL AUTO_INCREMENT,
  `device` int NOT NULL DEFAULT '0',
  `CodiceBadge` int NOT NULL DEFAULT '0',
  `entrata_uscita` int NOT NULL DEFAULT '0',
  `Valido` int DEFAULT NULL,
  `date` timestamp NULL DEFAULT NULL,
  `IdBadge` int DEFAULT NULL,
  PRIMARY KEY (`IdTransazioni`)
) ENGINE=InnoDB AUTO_INCREMENT=12807 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- L’esportazione dei dati non era selezionata.

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IFNULL(@OLD_FOREIGN_KEY_CHECKS, 1) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40111 SET SQL_NOTES=IFNULL(@OLD_SQL_NOTES, 1) */;
