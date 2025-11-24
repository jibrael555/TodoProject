#include <fmt/core.h>
#include <mysqlx/xdevapi.h>
#include <iostream>
#include <cctype>


//auto todoTable = connectToDB("localhost", "root", "root");

//mysqlx::Table connectToDB(std::string host, std::string user, std::string password) {
//	try {
//		mysqlx::Session session(host, user, password);
//		mysqlx::Schema schema = session.getSchema("todo"); // nama db
//		auto todoTable = schema.getTable("todos");
//		return todoTable;
//
//	} catch (mysqlx::Error e) {
//		std::cerr << e.what() << std::endl;
//	} catch (std::exception e) {
//		std::cerr << "Error: " << e.what() << std::endl;
//	}
//}

//int main() {
//	auto host = "localhost",
//		user = "root",
//		password = "root";
//	try {
//		mysqlx::Session session(host, user, password);
//		mysqlx::Schema schema = session.getSchema("todo"); // nama db
//		mysqlx::Table todoTable = schema.getTable("todos");
//		std::cout << "success" << std::endl;
//		return 0;
//	}
//	catch (mysqlx::Error e) {
//		std::cout << e.what() << std::endl;
//	}
//
//	return 0;
//}


void displayTodoType(mysqlx::Session &session) {
	auto result = session.sql("select id, nama from todo_types;").execute();
	auto rows = result.fetchAll();
	std::cout << "Tipe Todo yang tersedia:\n";

	for (const auto& row : rows) {
		int type_id = static_cast<int>(row[0]);
		std::string type_name = static_cast<std::string>(row[1]);
		fmt::print("ID: {}, Name: {}\n", type_id, type_name);
	}
}

int GetInputInt(const std::string prompt, const std::string invalid_massage) {
	bool is_valid = false;
	std::string input = "";
	int number = 0;

	while (!is_valid) {
		std::cout << prompt;
		std::getline(std::cin>>std::ws, input, '\n');

		if (!std::cin) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << invalid_massage << std::endl;
			continue;
		}

		try {
			number = std::stoi(input);
			std::cin.clear();
			is_valid = true;
		}
		catch (std::invalid_argument a) {
			std::cout << "tolong masukkan angka yang valid.\n";
		}
	}
	return number;
}

std::string GetInput(const std::string prompt, const std::string invalid_massage) {
	bool is_valid = false;
	std::string input;

	while (!is_valid) {
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cout << prompt;
		std::getline(std::cin, input);

		if (!std::cin) {
			std::cout << invalid_massage << std::endl;
			continue;
		}

		is_valid = true;
	}
	return input;
}

// returns true if user answers with 'y' ||'Y'
bool confirm(std::string prompt) {
	bool is_valid = false;
	char input;

	while (!is_valid) {
		std::cin.clear();

		std::cout << prompt;
		std::cin >> input; // TODO: handle \n
		
		input = std::tolower(input);
		
		bool is_invalid_input = (input != 'y' && input != 'n');
		
		if (!std::cin||is_invalid_input) {
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "invalid input, please answer with y/n." << std::endl;
			continue;
		}

		is_valid = true;
	}

	return input == 'y';
}

struct User {
	int id;
	std::string nama;
	std::string email;
	std::string password;
};

struct DatabaseCredentials {
	std::string host;
	std::string username;
	std::string password;
};

int main() {
	DatabaseCredentials creds;
	creds.host = "localhost";
	creds.username = "root";
	creds.password = "root";

	mysqlx::Session session(creds.host, creds.username, creds.password);
	mysqlx::Schema schema = session.getSchema("todo"); // nama db
	mysqlx::Table todoTable = schema.getTable("todos");
	mysqlx::Table todoTypesTable = schema.getTable("todo_types");
	
	mysqlx::Table userTable = schema.getTable("users");
	
	session.sql("use todo;").execute();

	bool login = true;
	User user;

	std::cout << "===== WELCOME =====\n";
	std::cout << "1. Register\n";
	std::cout << "2. Log in\n";
	std::cout << "3. Exit program\n";

	do {
		int option = GetInputInt("masukkan option: ", "Input Invalid!");
		
		switch (option) {
			case 1: {
				bool emailsama;
				do {
					std::cout << "Masukan nama : "; std::cin >> user.nama;
					bool pass=true;

					do {
						std::cout << "Masukan password (Minimal 8 karakter) : "; std::cin >> user.password;
						if (user.password.length() < 8) {
							std::cout << "Panjang password minimal 8 karakter!\n";
						}
						else {
							pass = false;
						}
					} while (pass);

					std::cout << "Masukan email : "; std::cin >> user.email;
					emailsama = false;

					try {
					auto result = userTable
						.insert("nama", "password", "email")
						.values(user.nama, user.password, user.email)
						.execute();
					}

					catch (mysqlx::Error e) {
						std::string reason = e.what();
						if (reason.find("Duplicate entry") != std::string::npos) {
							std::cout << "duplicate email. Choose another email" << std::endl;
							emailsama = true;
						}
					}

				
				} while (emailsama==true);
				std::cout << "Account registered successfully. Silahkan login\n";
				break;
			} 
			case 2: {
				std::cout << "Masukan email : ";
				std::cin >> user.email;
				std::cout << "Masukan password : ";
				std::cin >> user.password;
				
				auto result = userTable
					.select("id", "nama", "email", "password")
					.where("email = :email AND password = :password")
					.bind("email", user.email)
					.bind("password", user.password)
					.execute();
				auto row = result.fetchOne();

				if (row.isNull()) {
					std::cout << "Login failed. Invalid email or password.\n\n";
				}
				else {
					user.id = static_cast<int>(row[0]);
					user.nama = static_cast<std::string>(row[1]);
					user.email = static_cast<std::string>(row[2]);
					user.password = static_cast<std::string>(row[3]);
					std::cout << "Login successful. Welcome " << static_cast<std::string>(user.nama) << "!\n\n";
					login = false;
				}
				break;
			}
			case 3: {
				std::cout << "Exiting...\n";
				return 0;
			}
			default: {
				std::cout << "Input Invalid!\n";
				break;
			}
		}
		
	} while (login);

	int option=0,
		id=0;
	do {
		std::cout << "===== TODO LIST MENU =====\n";
		std::cout << "1. View Todos\n";
		std::cout << "2. Add Todo\n";
		std::cout << "3. Delete Todo\n";
		std::cout << "4. Update Todo\n";
		std::cout << "5. Add Todo Types\n";
		std::cout << "6. Delete Todo Types\n";
		std::cout << "7. Update Todo Types\n";
		std::cout << "8. Exit\n";

		std::cout << "Option : "; std::cin >> option;
		std::cout << std::endl;

		switch (option) {
		case 1: {
			auto result = session.sql("select td.id, td.name, td.is_done, ty.nama as type from todos td left join todo_types ty on td.todo_types_id = ty.id where td.user_id = ?")
				.bind(user.id)
				.execute();
			auto rows = result.fetchAll();

			if (result.count() == 0) {
				std::cout << "Todo belum ada" << std::endl;
				break;
			}

			for (const auto& row:rows) {
				int id = static_cast<int>(row[0]);
				std::string name = static_cast<std::string>(row[1]);
				bool is_done = static_cast<bool>(row[2]);
				std::string type;
				if (row[3].isNull()) {
					type = "Kosong";
				}
				else {
					type = static_cast<std::string>(row[3]);
				}
				fmt::print("ID: {}, Name: {}, is done: {}, Type: {}\n", id, name, is_done, type);
			}

			break;
		}

		case 2: {
			std::string namatodo = GetInput("Masukkan Nama Todo : ", "Masukan invalid!");
			
			auto result = todoTable.insert("name","user_id").values(namatodo, user.id).execute();
			std::cout << "Sukses menambahkan todo : " << namatodo << "\n\n";
			break;
		}

		case 3: {
			std::cout << "Masukan id todo yang ingin dihapus: "; std::cin >> id;	
			auto result = todoTable
				.remove()
				.where("id = :id AND user_id = :user_id")
				.bind("id", id)
				.bind("user_id", user.id)
				.execute();

			const int count = static_cast<int>(result.getAffectedItemsCount());
			
			if (count == 0) {
				std::cout << "gagal menghapus todo." << std::endl;
			}
			else {
				std::cout << "Sukses menghapus todo dengan id : " << id << "\n\n";
			}
			break; 
		}
		case 4: {
			int id = GetInputInt("Masukkan id todo yang ingin diupdate : ", "ID tidak valid!");
			
			mysqlx::Row row = todoTable
				.select("user_id")
				.where("id = :id")
				.bind("id", id)
				.execute()
				.fetchOne();
			auto targetTodoUserId = static_cast<int>(row.get(0));
			
			if (targetTodoUserId != user.id) {
				std::cout << "kamu ga boleh ngedit yang bukan punya kamu" << std::endl;
				break;
			}

			std::string namaTodo = GetInput("Masukkan nama baru (biarkan kosong jika tidak ingin update): ", "Nama tidak valid!");
			if (!namaTodo.empty()) {
				todoTable
					.update()
					.set("name", namaTodo)
					.where("id = :id")
					.bind("id", id)
					.execute();
			}

			displayTodoType(session);

			int todo_type_id_input = GetInputInt("Masukkan todo type id baru(biarkan kosong jika tidak ingin update) :", "");

			// handle invalid todo_type_id_input

			if (todo_type_id_input != 0) {
				todoTable
					.update()
					.set("todo_types_id", todo_type_id_input)
					.where("id = :id")
					.bind("id", id)
					.execute();
			}
			
			bool selesai = confirm("Tandai kalau sudah selesai y/n: ");

			todoTable
				.update()
				.set("is_done", selesai)
				.where("id = :id")
				.bind("id",id)
				.execute();

			break; 
		}
		case 5: {
			std::string namatypes = GetInput("Masukkan Nama Todo Type : ", "Masukan invalid!");
			auto result = todoTypesTable.insert("nama", "user_id").values(namatypes, user.id).execute();
			std::cout << "Sukses menambahkan todo : " << namatypes << "\n\n";
			break;
		}
		case 6: {
			int idtodo;
			std::cout << "Masukkan id Todo Types yang ingin dihapus: "; std::cin >> idtodo;

			auto result = session.sql("select name, is_done from todos where todo_types_id=?")
				.bind(idtodo)
				.execute();
			auto rows = result.fetchAll();

			if (result.count() == 0) {
				std::cout << "Todo belum ada" << std::endl;
			}
			else {
				for (const auto& row : rows) {
					int id = static_cast<int>(row[0]);
					std::string name = static_cast<std::string>(row[1]);
					bool is_done = static_cast<bool>(row[2]);
					std::string type;
					if (row[3].isNull()) {
						type = "Kosong";
					}
					else {
						type = static_cast<std::string>(row[3]);
					}
					fmt::print("ID: {}, Name: {}, is done: {}, Type: {}\n", id, name, is_done, type);
				}
			}

			bool yakin_hapus = confirm("Menghapus Todo Types akan menghapus semua Todo ini. Yakin ingin hapus? (y/n): ");

			if (yakin_hapus) {
				auto result = todoTable
					.remove()
					.where("todo_types_id = :idtodo AND user_id = :user_id")
					.bind("idtodo", idtodo)
					.bind("user_id", user.id)
					.execute();

				const int count = static_cast<int>(result.getAffectedItemsCount());

				if (count == 0) {
					std::cout << "gagal menghapus todo." << std::endl;
				}
				else {
					std::cout << "Sukses menghapus todo dengan id : " << id << "\n\n";
				}
			}
			
		}
		case 8:
			std::cout << "Exiting...\n";
			return 0;
			break;
		default:
			std::cout << "Invalid option\n";
			break;
		}
		bool mau_lanjut = confirm("Mau lanjut? (y/n): ");

		if (!mau_lanjut) {
			std::cout << "Exiting...\n";
			return 0;
		}
		else {
			system("cls");
		} 
	} while (option != 5);
	return 0;
}