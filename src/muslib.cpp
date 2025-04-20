#include <cstdlib>
#include <soci/soci.h>
#include <soci/mysql/soci-mysql.h>
#include <iostream>
#include <string>

#define show(x) std::cout << x << std::endl

class Song
{
public:
     int id;
     std::string title;
     std::string author;
     std::string tonality;
     std::string lyrics;
     Song() : id(0) {}
     Song(const std::string& i, const std::string& a, const std::string& t, const std::string& l)
	  : title(i), author(a), tonality(t), lyrics(l) {}

     void sing() const {
	  std::cout << "id: " << id << "\n"
		    << "title: " << title << "\n"
		    << "author: " << author << "\n"
		    << "tonality: " << (tonality.empty() ? "N/A" : tonality) << "\n"
		    << "lyrics: " << (lyrics.empty() ? "N/A" : lyrics) << "\n"
		    << "-------------------------\n";
     }

     void overview() const {
	  std::cout << id << ". " << title << " by " << author << " in " << tonality << std::endl;
     }

     friend struct soci::type_conversion<Song>;
};


namespace soci
{
     template<>
     struct type_conversion<Song>
     {
	  typedef values base_type;
	  static void from_base(values const & v, indicator ind, Song & s) {

	       if (ind == i_null) return;
	       s.id = v.get<int>("id");
	       s.title = v.get<std::string>("title");
	       s.author = v.get<std::string>("author");
	       s.tonality = v.get<std::string>("tonality", "");
	       s.lyrics = v.get<std::string>("lyrics", "");
	  }
	  static void to_base(Song const & s, values & v, indicator & ind) {

	       v.set("id", s.id);
	       v.set("title", s.title);
	       v.set("author", s.author);
	       v.set("tonality", s.tonality);
	       v.set("lyrics", s.lyrics);
	       ind = i_ok;
	  }
     };
}


void create_table_if_not_exists(soci::session& sql)
{
     sql << "CREATE TABLE IF NOT EXISTS song ("
	 << "id INT AUTO_INCREMENT PRIMARY KEY, "
	 << "title VARCHAR(255), "
	 << "author VARCHAR(255), "
	 << "tonality VARCHAR(255), "
	 << "lyrics TEXT)";
}


std::vector<Song> select_star(soci::session& sql)
{
     std::vector<Song> all;
     soci::rowset<Song> rs = (sql.prepare << "SELECT * FROM song");
     for (const auto & song : rs) all.push_back(song);
     return all;
}


Song select_id(soci::session& sql, int id)
{
     Song song;
     sql << "SELECT * FROM song WHERE id = :id", soci::into(song), soci::use(id);
     return song;
}


void update_set(soci::session & sql, const Song & song)
{
     sql << "UPDATE song SET "
	  "title = :title, "
	  "author = :author, "
	  "tonality = :tonality, "
	  "lyrics = :lyrics "
	  "WHERE id = :id",
	  soci::use(song);
}


int insert_song(soci::session& sql, Song & song)
{
     sql << "INSERT INTO song(title, author, tonality, lyrics) "
	  "VALUES(:title, :author, :tonality, :lyrics)", soci::use(song);
     sql << "SELECT LAST_INSERT_ID()", soci::into(song.id);
     return song.id;
}


void delete_id(soci::session & sql, int id)
{
     sql << "DELETE FROM song WHERE id = :id", soci::use(id);
}


void showSong(soci::session& sql)
{
     int id;
     std::cout << "enter id: ";
     std::cin >> id;
     Song song = select_id(sql, id);
     song.sing();
}


void listAll(soci::session& sql)
{
     std::vector<Song> all = select_star(sql);
     show("\nindex, title, author, tonality");
     for (const Song & s : all) s.overview();
}


void dumpAll(soci::session& sql)
{
     std::vector<Song> all = select_star(sql);
     for (const Song & s : all) s.sing();
}


void newSong(soci::session& sql)
{
     std::string title, author, tonality, lyrics;
     std::cout << "enter title: ";
     getline(std::cin, title);
     std::cout << "enter author: ";
     getline(std::cin, author);
     std::cout << "enter tonality: ";
     getline(std::cin, tonality);
     std::cout << "enter lyrics: ";
     getline(std::cin, lyrics);

     Song song(title, author, tonality, lyrics);
     int id = insert_song(sql, song);
     Song verify = select_id(sql, id);
     verify.sing();
}


void deleteSong(soci::session& sql)
{
     int id;
     std::cout << "enter id: ";
     std::cin >> id;
     delete_id(sql, id);
     listAll(sql);
}


int main()
{
     const char * db_pass = getenv("MUSLIB_PASS");
     if (!db_pass)
     {
	  std::cerr << "warning: MUSLIB_PASS has not been set, try\n\n > export MUSLIB_PASS=<user_password>\n" << std::endl;
	  return 0;
     }
     std::string connection_string = "db=muslib user=kalo password=" + std::string(db_pass) + " host=localhost";
     try
     {
	  soci::session sql(soci::mysql, connection_string); 
	  create_table_if_not_exists(sql);
	  listAll(sql);
	  while (true)
	  {
	       std::cout << "\nmenu: 0. show song, 1. add song, 2. delete song, 3. list, 4. dump, 5. quit\nchoice: ";
	       int choice;
	       std::cin >> choice;
	       std::cin.ignore();
	       if (choice == 0) showSong(sql);
	       else if (choice == 1) newSong(sql);
	       else if (choice == 2) deleteSong(sql);
	       else if (choice == 3) listAll(sql);
	       else if (choice == 4) dumpAll(sql);
	       else break;
	  }	  
     } catch (const soci::soci_error& e)
     {
	  std::cerr << "database error: " << e.what() << std::endl;
     }
     return 0;
}
