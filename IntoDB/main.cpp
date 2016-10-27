#include <cstdlib>
#include <iostream>

/*
Include directly the different
headers from cppconn/ and mysql_driver.h + mysql_util.h
(and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "MotionParser.h"

using namespace std;

int main(int argc, char** argv){
	MotionParser mp;
	mp.parse("../data/motion_xy.csv");
	vector<Pos> pos_lst = mp.getPosition();

	try {
		sql::Driver *driver;
		sql::Connection *con;
		sql::Statement *stmt;
		sql::ResultSet *res;
		sql::PreparedStatement *pstmt;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "root");
		/* Connect to the MySQL test database */
		con->setSchema("motiontrail");

		stmt = con->createStatement();
		stmt->execute("DROP TABLE IF EXISTS path");
		stmt->execute("CREATE TABLE path ("
		"id INTEGER PRIMARY KEY AUTO_INCREMENT,"
		"height INT,"
		"x DECIMAL(18,6),"
		"y DECIMAL(18,6),"
		"state INT)");
		delete stmt;

		con->setAutoCommit(false);
		/* '?' is the supported placeholder syntax */
		pstmt = con->prepareStatement("INSERT INTO path (height, x, y, state) VALUES (?, ?, ?, ?)");
		for (int i = 1; i <= pos_lst.size(); i++) {
			pstmt->setInt(1,pos_lst[i-1].h);
			pstmt->setDouble(2,pos_lst[i-1].x);
			pstmt->setDouble(3,pos_lst[i-1].y);
			pstmt->setInt(4, (int)pos_lst[i-1].state);
			pstmt->executeUpdate();
		}
		delete pstmt;
		con->commit();

		/* Select in ascending order */
		pstmt = con->prepareStatement("SELECT id FROM path ORDER BY id ASC");
		res = pstmt->executeQuery();

		/* Fetch in reverse = descending order! */
		res->afterLast();
		while (res->previous())
			cout << "\t... MySQL counts: " << res->getInt("id") << endl;
		delete res;

		delete pstmt;
		delete con;

	}
	catch (sql::SQLException &e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}

	cout << endl;

	return EXIT_SUCCESS;
}