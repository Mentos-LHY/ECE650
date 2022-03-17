#include "query_funcs.h"

void exe_com_SQL(connection * C, string sql) {
  work W(*C);
  W.exec(sql);
  W.commit();
}

/////////////////////////////////////////////////////////////////////
void add_player(connection * C,
                int team_id,
                int jersey_num,
                string first_name,
                string last_name,
                int mpg,
                int ppg,
                int rpg,
                int apg,
                double spg,
                double bpg) {
  stringstream sql;
  work W(*C);
  sql << "INSERT INTO PLAYER (TEAM_ID, UNIFORM_NUM, FIRST_NAME, LAST_NAME, MPG, PPG, "
         "RPG, APG, SPG, BPG) VALUES ("
      << team_id << ", " << jersey_num << ", " << W.quote(first_name) << ", "
      << W.quote(last_name) << ", " << mpg << ", " << ppg << ", " << rpg << ", " << apg
      << ", " << spg << ", " << bpg << ");";

  //exe_com_SQL(C, sql.str()); // Because I need to use W.quote(), I can not abstract this method out
  W.exec(sql.str());
  W.commit();
}

void add_team(connection * C,
              string name,
              int state_id,
              int color_id,
              int wins,
              int losses) {
  stringstream sql;
  work W(*C);
  sql << "INSERT INTO TEAM (NAME, STATE_ID, COLOR_ID, WINS, LOSSES) VALUES ("
      << W.quote(name) << ", " << state_id << ", " << color_id << ", " << wins << ", "
      << losses << ");";
  //exe_com_SQL(C, sql.str());
  W.exec(sql.str());
  W.commit();
}

void add_state(connection * C, string name) {
  stringstream sql;
  work W(*C);
  sql << "INSERT INTO STATE (NAME) VALUES (" << W.quote(name) << ");";

  // exe_com_SQL(C, sql.str());
  W.exec(sql.str());
  W.commit();
}

void add_color(connection * C, string name) {
  stringstream sql;
  work W(*C);
  sql << "INSERT INTO COLOR (NAME) VALUES (" << W.quote(name) << ");";
  //exe_com_SQL(C, sql.str());
  W.exec(sql.str());
  W.commit();
}

////////////////////////////////////////////////////////////////////////
void drop_table(connection * C, string table_name) {
  string sql = "DROP TABLE IF EXISTS " + table_name + " CASCADE;";
  exe_com_SQL(C, sql);
  //cout << "Drop table " + table_name << endl;
}

void create_tables(connection * C, string f_name) {
  string line, sql;

  ifstream f(f_name.c_str());
  if (f.is_open()) {
    while (getline(f, line)) {
      sql.append(line);
    }
    f.close();
  }
  else {
    cerr << "Cannot open file " + f_name << endl;
    return;
  }

  exe_com_SQL(C, sql);
  //cout << "Create tables" << endl;
}

void setup_tables(connection * C) {
  drop_table(C, "PLAYER");
  drop_table(C, "TEAM");
  drop_table(C, "STATE");
  drop_table(C, "COLOR");
  create_tables(C, "tableSetup.sql");
}

/////////////////////////////////////////////////////////////////////////
void populate_STATE(connection * C, string f_name) {
  string line, name;
  int state_id;

  ifstream f(f_name.c_str());
  if (f.is_open()) {
    while (getline(f, line)) {
      stringstream ss;
      ss << line;
      ss >> state_id >> name;
      add_state(C, name);
    }
    f.close();
  }
  else {
    cerr << "Cannot open file " + f_name << endl;
    return;
  }
  // cout << "Populate table STATE successfully" << endl;
}

void populate_COLOR(connection * C, string f_name) {
  string line, name;
  int color_id;

  ifstream f(f_name.c_str());
  if (f.is_open()) {
    while (getline(f, line)) {
      stringstream ss;
      ss << line;
      ss >> color_id >> name;
      add_color(C, name);
    }
    f.close();
  }
  else {
    cerr << "Cannot open file " + f_name << endl;
    return;
  }
  //cout << "Populate table COLOR successfully" << endl;
}

void populate_TEAM(connection * C, string f_name) {
  string line, name;
  int team_id, state_id, color_id, wins, losses;

  ifstream f(f_name.c_str());
  if (f.is_open()) {
    while (getline(f, line)) {
      stringstream ss;
      ss << line;
      ss >> team_id >> name >> state_id >> color_id >> wins >> losses;
      add_team(C, name, state_id, color_id, wins, losses);
    }
    f.close();
  }
  else {
    cerr << "Cannot open file " + f_name << endl;
    return;
  }
  //cout << "Populate table TEAM successfully" << endl;
}

void populate_PLAYER(connection * C, string f_name) {
  string line, first_name, last_name;
  int player_id, team_id, jersey_num, mpg, ppg, rpg, apg;
  double spg, bpg;

  ifstream f(f_name.c_str());
  if (f.is_open()) {
    while (getline(f, line)) {
      stringstream ss;
      ss << line;
      ss >> player_id >> team_id >> jersey_num >> first_name >> last_name >> mpg >> ppg >>
          rpg >> apg >> spg >> bpg;
      add_player(
          C, team_id, jersey_num, first_name, last_name, mpg, ppg, rpg, apg, spg, bpg);
    }
    f.close();
  }
  else {
    cerr << "Cannot open file " + f_name << endl;
    return;
  }
  //cout << "Populate table PLAYER successfully" << endl;
}

void populate_tables(connection * C) {
  populate_STATE(C, "state.txt");
  populate_COLOR(C, "color.txt");
  populate_TEAM(C, "team.txt");
  populate_PLAYER(C, "player.txt");
}
/////////////////////////////
void query1(connection * C,
            int use_mpg,
            int min_mpg,
            int max_mpg,
            int use_ppg,
            int min_ppg,
            int max_ppg,
            int use_rpg,
            int min_rpg,
            int max_rpg,
            int use_apg,
            int min_apg,
            int max_apg,
            int use_spg,
            double min_spg,
            double max_spg,
            int use_bpg,
            double min_bpg,
            double max_bpg) {
  stringstream sql;
  sql << "SELECT * FROM PLAYER";

  int low_bound_1st[4] = {min_mpg, min_ppg, min_rpg, min_apg};
  int up_bound_1st[4] = {max_mpg, max_ppg, max_rpg, max_apg};
  double low_bound_2nd[2] = {min_spg, min_bpg};
  double up_bound_2nd[2] = {max_spg, max_bpg};

  int enable[6] = {use_mpg, use_ppg, use_rpg, use_apg, use_spg, use_bpg};
  string attribute[6] = {"MPG", "PPG", "RPG", "APG", "SPG", "BPG"};

  nontransaction N(*C);

  int en_flag = 0;
  for (int i = 0; i < 4; i++) {
    if (enable[i] != 0) {
      if (en_flag == 0) {
        sql << " WHERE ";
      }
      if (en_flag != 0) {
        sql << " AND ";
      }
      sql << attribute[i] << " >= " << low_bound_1st[i] << " AND " << attribute[i]
          << " <= " << up_bound_1st[i];
      en_flag = 1;
    }
  }
  for (int i = 0; i < 2; i++) {
    if (enable[4 + i] != 0) {
      if (en_flag == 0) {
        sql << "WHERE ";
      }
      if (en_flag != 0) {
        sql << " AND ";
      }
      sql << attribute[4 + i] << " >= " << low_bound_2nd[i] << " AND " << attribute[4 + i]
          << " <= " << up_bound_2nd[i];
      en_flag = 1;
    }
  }
  sql << ";";

  result R(N.exec(sql.str()));

  cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG SPG BPG"
       << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<int>() << " " << c[1].as<int>() << " " << c[2].as<int>() << " "
         << c[3].as<string>() << " " << c[4].as<string>() << " " << c[5].as<int>() << " "
         << c[6].as<int>() << " " << c[7].as<int>() << " " << c[8].as<int>() << " "
         << fixed << setprecision(1) << c[9].as<double>() << " " << c[10].as<double>()
         << endl;
  }
}

void query2(connection * C, string uniform_color) {
  stringstream sql;
  sql << "SELECT TEAM.NAME FROM TEAM, COLOR WHERE TEAM.COLOR_ID = COLOR.COLOR_ID AND "
         "COLOR.NAME = \'"
      << uniform_color << "\';";

  nontransaction N(*C);
  result R(N.exec(sql.str()));
  cout << "NAME" << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << endl;
  }
}

void query3(connection * C, string team_name) {
  stringstream sql;
  sql << "SELECT FIRST_NAME, LAST_NAME FROM PLAYER, TEAM WHERE PLAYER.TEAM_ID = "
         "TEAM.TEAM_ID AND TEAM.NAME = \'"
      << team_name << "\' ORDER BY PPG DESC;";

  nontransaction N(*C);
  result R(N.exec(sql.str()));
  cout << "FIRST_NAME LAST_NAME" << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << " " << c[1].as<string>() << endl;
  }
}

void query4(connection * C, string state_name, string uniform_color) {
  stringstream sql;
  sql << "SELECT FIRST_NAME, LAST_NAME, UNIFORM_NUM FROM PLAYER, STATE, COLOR, TEAM "
         "WHERE ";
  sql << "PLAYER.TEAM_ID = TEAM.TEAM_ID AND TEAM.COLOR_ID = COLOR.COLOR_ID AND "
         "TEAM.STATE_ID = STATE.STATE_ID AND ";
  sql << "STATE.NAME = \'" << state_name << "\' AND "
      << "COLOR.NAME = \'" << uniform_color << "\';";

  nontransaction N(*C);
  result R(N.exec(sql.str()));
  cout << "FIRST_NAME LAST_NAME UNIFORM_NUM" << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << " " << c[1].as<string>() << " " << c[2].as<int>()
         << endl;
  }
}

void query5(connection * C, int num_wins) {
  stringstream sql;
  sql << "SELECT FIRST_NAME, LAST_NAME, NAME, WINS FROM PLAYER, TEAM WHERE ";
  sql << "PLAYER.TEAM_ID = TEAM.TEAM_ID AND WINS >" << num_wins << ";";

  nontransaction N(*C);
  result R(N.exec(sql.str()));
  cout << "FIRST_NAME LAST_NAME NAME WINS" << endl;
  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    cout << c[0].as<string>() << " " << c[1].as<string>() << " " << c[2].as<string>()
         << " " << c[3].as<int>() << endl;
  }
}
