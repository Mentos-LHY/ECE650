#include <iostream>
#include <pqxx/pqxx>

using namespace std;
using namespace pqxx;

int main(int argc, char * argv[]) {
  try {
    connection C("dbname=testdb user=postgres password=abc123");
    if (C.is_open()) {
      cout << "Opened database successfully: " << C.dbname() << endl;
    }
    else {
      cerr << "Can't open database" << endl;
      return 1;
    }
    C.disconnect();
  }
  catch (const std::exception & e) {
    cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
