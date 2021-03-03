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

-- Dump della struttura di tabella acap100.config
CREATE TABLE IF NOT EXISTS `config` (
  `Gate` int NOT NULL DEFAULT '0',
  `DeviceName` mediumtext NOT NULL,
  `ssidSD` mediumtext NOT NULL,
  `passSD` mediumtext NOT NULL,
  `MacAddress` mediumtext NOT NULL,
  `local_IP` tinytext NOT NULL,
  `gateway` tinytext NOT NULL,
  `subnet` tinytext NOT NULL,
  `primaryDNS` tinytext NOT NULL,
  `secondaryDNS` tinytext NOT NULL,
  `DBAddressParam` tinytext NOT NULL,
  `DBPortParam` int NOT NULL DEFAULT '3306',
  `BeepOnOff` int NOT NULL DEFAULT '0',
  `DBUserParam` tinytext NOT NULL,
  `DBPswParam` tinytext NOT NULL,
  `SyncDB` int NOT NULL DEFAULT '120',
  `SingleGate` int DEFAULT '1',
  `TimeGate1` int DEFAULT '1',
  `TimeGate2` int DEFAULT '1',
  `tftTimeout` int DEFAULT '0',
  `tzOffset` int DEFAULT '3600',
  `TransitUpdate` int DEFAULT '300',
  `Log` int DEFAULT '0',
  `DelLog` int DEFAULT '0',
  `DataBaseName` tinytext,
  `NTPUpDate` int DEFAULT '4',
  PRIMARY KEY (`Gate`) USING BTREE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;

-- L’esportazione dei dati non era selezionata.

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IFNULL(@OLD_FOREIGN_KEY_CHECKS, 1) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40111 SET SQL_NOTES=IFNULL(@OLD_SQL_NOTES, 1) */;
