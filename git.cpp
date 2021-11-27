#include<fstream>
#include<fcntl.h>
#include<dirent.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<bits/stdc++.h>

#include"sha1.hpp"

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define RESET "\033[0m"

using namespace std;

// Shows errors
void showError(string str) {
	perror(&str[0]);
	exit(0);
}

// Check if a directory exists or not
bool isDirExist(const string& path) {
	struct stat info;
	if (stat(path.c_str(), &info) != 0) {
		return false;
	}
	return (info.st_mode & S_IFDIR) != 0;
}

// Check if the given path is a file and has some data in it
bool checkFileSize(string path) {
	struct stat fileStat;
	if (stat(&path[0], &fileStat)) {
		return false;
	}
	if (!S_ISREG(fileStat.st_mode)) {
		return false;
	}
	// If no data in add.txt then returns 0
	return fileStat.st_size;
}

// Returns no.of directories in a given directory
int countDirectories(string path) {
	DIR* dir = opendir(&path[0]);
	struct dirent* direntp;
	int count = 0;
	// Directory exists
	if (dir == NULL)
		return count;

	while ((direntp = readdir(dir)) != NULL) {
		if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0)
			continue;
		if (direntp->d_type == DT_DIR) {
			count++;
		}
	}
	closedir(dir);
	return count;
}

// Tokenize the given string into words
vector<string> splitString(string& str) {
	istringstream iss(str);
	vector<string> result;
	string word;
	while (iss >> word) {
		result.push_back(word);
	}
	return result;
}

// Read line by line from the file
vector<string> readFile(string file_name) {
	// Reading the file
	vector<string> result;
	ifstream ifs(file_name, ifstream::in);
	for (string line; getline(ifs, line);) {
		result.push_back(line);
	}
	// Closing the file
	ifs.close();
	return result;
}

unordered_set<string> readFileSet(string file_name) {
	// Reading the file
	unordered_set<string> result;
	ifstream ifs(file_name, ifstream::in);
	for (string line; getline(ifs, line);) {
		result.insert(line);
	}
	// Closing the file
	ifs.close();
	return result;
}

unordered_map<string, string> readFileMap(string file_name) {
	//read status.txt file
	ifstream ifs;
	ifs.open(file_name);
	unordered_map<string, string> result;
	while (!ifs.eof()) {
		string line, stData, data;
		int flag;
		if (getline(ifs, line)) {
			vector<string> words = splitString(line);
			result[words[0]] = words[1];
		}
	}
	ifs.close();
	return result;
}

// Copies file from source to destination
void copyFile(string source, string destination) {
	// cout << source << " " << destination << endl;
	char buff[BUFSIZ];
	FILE* src = fopen(&source[0], "r");
	FILE* dest = fopen(&destination[0], "w");
	size_t size;

	// Copying the contents
	while ((size = fread(buff, 1, BUFSIZ, src)) > 0) {
		fwrite(buff, 1, size, dest);
	}
	// Copying the permissions
	struct stat st;
	stat(&source[0], &st);
	chmod(&destination[0], st.st_mode);
	fclose(src);
	fclose(dest);
}

// Copies Driectory from path to destination
void copyDirectory(char* path, char* des) {
	int status = mkdir(des, S_IRUSR | S_IWUSR | S_IXUSR);
	DIR* d;
	struct dirent* dir;
	d = opendir(path);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if ((string(dir->d_name) == "..") || (string(dir->d_name) == ".")) {
				continue;
			}
			else {
				string finalpath = string(path) + "/" + string(dir->d_name);
				char* newpath = new char[finalpath.length() + 1];
				strcpy(newpath, finalpath.c_str());

				string finaldestpath = string(des) + "/" + string(dir->d_name);
				char* newdestpath = new char[finaldestpath.length() + 1];
				strcpy(newdestpath, finaldestpath.c_str());

				struct stat sb;
				if (stat(newpath, &sb) == -1) {
					perror("lstat");
				}
				else {
					if ((S_ISDIR(sb.st_mode))) {
						copyDirectory(newpath, newdestpath);
					}
					else {
						copyFile(newpath, newdestpath);
					}
				}
			}
		}
	}
	else {
		showError("No such Directory found while copying with path :::::" + string(path));
	}
}

// Takes input as directory path and output vector reference 
void listFiles(const char* dirname, vector<string>& files) {
	DIR* dir = opendir(dirname);
	// base condition
	if (dir == NULL) {
		return;
	}
	// structure that reads the directory contents
	struct dirent* entity;
	entity = readdir(dir);
	while (entity != NULL) {
		// Ignoring current, parent and .git directories
		if (strcmp(entity->d_name, ".") != 0 && strcmp(entity->d_name, "..") != 0 && strcmp(entity->d_name, ".git") != 0) {
			if (strcmp(dirname, ".") != 0) {
				string path = dirname;
				path += '/';
				path += entity->d_name;
				// Erase the first 2 characters of the file path .i.e remove "./"
				path.erase(path.begin(), path.begin() + 2);
				// NOT adding directory names
				if (!isDirExist(path))
					files.push_back(path);
			}
			// NOT adding directory names
			else if (entity->d_type == DT_REG) {
				files.push_back(entity->d_name);
			}
		}
		// Recursive function call if the dirent is a directory
		// Avoiding the current and previous directory from listing
		if (entity->d_type == DT_DIR && strcmp(entity->d_name, ".") != 0 && strcmp(entity->d_name, "..") != 0 && strcmp(entity->d_name, ".git") != 0) {
			// creating the new path with in the directory
			char path[100] = { 0 };
			strcat(path, dirname);
			strcat(path, "/");
			strcat(path, entity->d_name);
			listFiles(path, files);
		}
		entity = readdir(dir);
	}
	closedir(dir);
}

// Writing data to file
void writeData(vector<string> files, string dest) {
	ofstream output_file(dest);
	ostream_iterator<string> output_iterator(output_file, "\n");
	copy(files.begin(), files.end(), output_iterator);
}

// Creates nested directories 
bool makeDirectory(const string& path) {
	mode_t mode = 0777;
	if (mkdir(path.c_str(), mode) == 0)
		return true;
	// Parent directory didn't exist, try to create it
	if (errno == ENOENT) {
		int pos = path.find_last_of('/');
		if (pos == string::npos)
			return false;
		if (!makeDirectory(path.substr(0, pos)))
			return false;
		// Now parent exists, try to create again
		return mkdir(path.c_str(), mode) == 0;
	}
	// Directory already exists
	else if (errno == EEXIST) {
		return isDirExist(path);
	}
	return false;
}

// Update log file
void updateLog(string text) {
	// Open log file
	FILE* fp = fopen("./.git/log.txt", "a");
	fwrite(&text[0], 1, text.length(), fp);
	fclose(fp);
}

// Copy files into the new version directory
void addFilesToVersionDir(vector<string>& files, string destination) {
	// Create the version Directory
	// string destination = "./.git/version";
	// int countDir = countDirectories(destination);
	// destination += "/v_" + to_string(countDir);
	// if (mkdir(&destination[0], 0777) == -1) {
	// 	perror("Version Directory not created");
	// 	return;
	// }
	// Add files in the version directory
	// Read names from add.txt file
	for (int i = 0; i < files.size(); i++) {
		string source = files[i];
		// source: "test/test1/a.txt"
		// destination: "./.git/version/v_i"
		// First create the nested desination folders: ./.git/version/v_i/test/test1
		// Then copy file
		int pos = source.find_last_of('/');
		if (pos == string::npos) {
			makeDirectory(destination + "/");
		}
		else {
			makeDirectory(destination + "/" + source.substr(0, pos));
		}
		copyFile("./" + source, destination + "/" + source);
	}
}

// Delete file data
void deleteData(string file) {
	FILE* fp = fopen(&file[0], "w");
	fclose(fp);
}

void delete_file(string Destination) {
	unlink(Destination.c_str());
	return;
}

// Updates the status.txt file with the given files
void updateStatus(vector<string>& files) {
	// Add files and SHA1 values 
	vector<string> fileHashes;
	for (int i = 0; i < files.size(); i++) {
		string hash = SHA1::from_file(files[i]);
		fileHashes.push_back(files[i] + " " + hash);
	}
	string dest = "./.git/status.txt";
	writeData(fileHashes, dest);
}

// ================================================ commands ==========================================================

// Initializes git repo
void init() {
	char path[PATH_MAX];
	getcwd(path, PATH_MAX);
	string HomeDir;
	HomeDir = (string)path + "/.git";
	// Read, write, execute for owner (./git)
	mkdir(HomeDir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
	// version directory
	mkdir((HomeDir + "/version").c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
	// Creating log.txt, status.txt and add.txt
	open((HomeDir + "/log.txt").c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	open((HomeDir + "/status.txt").c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	open((HomeDir + "/add.txt").c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
}

void add(string file) {
	// read from add.txt
	string dest = "./.git/add.txt";
	vector<string> addFiles = readFile(dest);
	vector<string> files_to_be_added;
	unordered_set<string> addFilesSet;

	for (auto it : addFiles) {
		addFilesSet.insert(it);
	}

	// check if add.txt already consist of that files
	if (file == ".") {
		listFiles(".", files_to_be_added);
		for (auto it : files_to_be_added) {
			if (addFilesSet.find(it) != addFilesSet.end()) {
				addFilesSet.erase(it);
			}
		}
	}
	else {
		// If it's a directory
		if (isDirExist(file)) {
			string folder = "./" + file;
			listFiles(&folder[0], files_to_be_added);
		}
		// If it's a file
		else {
			files_to_be_added.push_back(file);
		}

		// Removing if the file is already in add.txt
		for (auto it : files_to_be_added) {
			if (addFilesSet.find(it) != addFilesSet.end()) {
				addFilesSet.erase(it);
			}
		}
	}
	// Now append the "add" files 
	for (auto it : addFilesSet) {
		files_to_be_added.push_back(it);
	}
	// Write the data
	writeData(files_to_be_added, dest);
}

void commit() {
	// Check if "git init" commnad is executed or not.
	// If there is .git folder in the current directory then it means "git init" was executed
	bool initDone = isDirExist("./.git");
	if (!initDone) {
		cout << "Not a git repository." << endl;
		return;
	}

	// Check if "git init" commnad is executed or not.
	// If there is .git folder in the current directory then it means "git init" was executed
	bool addDone = checkFileSize("./.git/add.txt");
	if (!addDone) {
		cout << "Nothing to commit" << endl;
		return;
	}

	// 1. Update the log.txt
	int countDir = countDirectories("./.git/version");
	time_t rawtime;
	time(&rawtime);
	string text = "commit\n";
	text += "version: v_" + to_string(countDir + 1) + "\n";
	text += "date: " + string(ctime(&rawtime)) + "\n\n";
	updateLog(text);

	// Read from status.txt 
	unordered_map<string, string> statusFiles = readFileMap("./.git/status.txt");
	// Read from add.txt
	vector<string> addFiles = readFile("./.git/add.txt");

	// 2. Create a new version folder and add the files in it
	string destination = "./.git/version/v_";
	destination += to_string(countDir + 1);

	if (mkdir(&destination[0], 0777) == -1) {
		perror("Version Directory not created");
		return;
	}

	char buff[PATH_MAX];
	getcwd(buff, PATH_MAX);
	string cwd = string(buff);

	// Find all files in the current directory
	vector<string> allFiles;
	listFiles(".", allFiles);
	unordered_set<string> allFilesSet;
	for (auto it : allFiles) {
		allFilesSet.insert(it);
	}

	vector<string> previous_version_files;
	// Add previous_version_files to the new version folder
	if (countDir != 0) {
		// Find the files in that directory
		string folder = "./.git/version/v_" + to_string(countDir);
		chdir(&folder[0]);
		listFiles(".", previous_version_files);
		chdir(&cwd[0]);
		vector<string> files;
		for (auto it : previous_version_files) {
			struct stat fileInfo;
			// File exists
			if (stat(&it[0], &fileInfo) == 0) {
				files.push_back(it);
			}
			else {
				statusFiles.erase(it);
			}
		}
		chdir(&folder[0]);
		string dest = "../v_" + to_string(countDir + 1);
		addFilesToVersionDir(files, dest);
		chdir(&cwd[0]);
	}

	// Now add the new/modified files 
	addFilesToVersionDir(addFiles, destination);

	// 3. Adding files to status.txt
	for (auto it : addFiles) {
		if (statusFiles.find(it) != statusFiles.end()) {
			statusFiles.erase(it);
		}
	}
	// Adding the previous version files to status.txt
	for (auto it : statusFiles) {
		addFiles.push_back(it.first);
	}
	updateStatus(addFiles);

	// 4. Delete add.txt data
	deleteData("./.git/add.txt");
}

void status() {
	vector<string> allFiles;
	listFiles(".", allFiles);

	unordered_set<string> addFiles = readFileSet("./.git/add.txt");
	unordered_map<string, string> statusFiles = readFileMap("./.git/status.txt");

	vector<string> untrackedFiles, to_be_committed_files, modified_files;

	for (auto it : allFiles) {
		// Untracked files i.e. not present in add.txt and not in status.txt
		if (addFiles.find(it) == addFiles.end() && statusFiles.find(it) == statusFiles.end()) {
			untrackedFiles.push_back(it);
			// cout << "(Untracked)\t" << it << endl;
		}
		string hash = SHA1::from_file(it);
		// Changes to be commited files
		if (addFiles.find(it) != addFiles.end()) {
			// cout << "(To be committed)\t" << it << endl;
			to_be_committed_files.push_back(it);
		}
		// Tracked and modified files
		else if (statusFiles.find(it) != statusFiles.end() && statusFiles[it] != hash) {
			modified_files.push_back(it);
			// cout << "(Modified)\t" << it << endl;
		}
	}
	for (auto it : untrackedFiles) {
		cout << KRED "(Untracked)\t" << it << RESET << endl;
	}
	for (auto it : to_be_committed_files) {
		cout << KGRN "(Staged)\t" << it << RESET << endl;
	}
	for (auto it : modified_files) {
		cout << KYEL "(Modified)\t" << it << RESET << endl;
	}
}

void push(string dest) {
	string push_loc = dest;
	mkdir(&push_loc[0], S_IRUSR | S_IWUSR | S_IXUSR);

	string src = "./.git/version";
	int count = countDirectories(src);
	src += "/v_" + to_string(count);

	copyDirectory(&src[0], &push_loc[0]);

	// Update the log.txt
	int countDir = countDirectories("./.git/version");
	time_t rawtime;
	time(&rawtime);
	string text = "commit\n";
	// text += "version: v_" + to_string(countDir + 1) + "\n";
	text += "date: " + string(ctime(&rawtime)) + "\n\n";
	updateLog(text);
}

//------------------------ Do Not Delete any Function even if it is repeated because they have minor changes ----------RollBack---

set<string>current_directory;
vector<dirent*> content;

void SplitString2(string s, vector<string>& v) {
	string temp = "";
	for (int i = 0;i < s.length();++i) {

		if (s[i] == '/') {
			v.push_back(temp);
			temp = "";
		}
		else {
			temp.push_back(s[i]);
		}

	}
	v.push_back(temp);
}

int remove_dir(const char* path) {
	DIR* d = opendir(path);
	size_t path_len = strlen(path);
	int r = -1;

	if (d) {
		struct dirent* p;

		r = 0;
		while (!r && (p = readdir(d))) {
			int r2 = -1;
			char* buf;
			size_t len;

			/* Skip the names "." and ".." as we don't want to recurse on them. */
			if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
				continue;

			len = path_len + strlen(p->d_name) + 2;
			buf = (char*)malloc(len);

			if (buf) {
				struct stat statbuf;

				snprintf(buf, len, "%s/%s", path, p->d_name);
				if (!stat(buf, &statbuf)) {
					if (S_ISDIR(statbuf.st_mode))
						r2 = remove_dir(buf);
					else
						r2 = unlink(buf);
				}
				free(buf);
			}
			r = r2;
		}
		closedir(d);
	}

	if (!r)
		r = rmdir(path);

	return r;
}

void copyFile_roll(string source, string destination, char* d_name) {
	// cout << source << " " << destination << endl;
	if (current_directory.find(string(d_name)) != current_directory.end()) {
		delete_file(&(destination)[0]);
	}
	char buff[BUFSIZ];
	FILE* src = fopen(&source[0], "r");
	FILE* dest = fopen(&destination[0], "w");
	size_t size;

	// Copying the contents
	while ((size = fread(buff, 1, BUFSIZ, src)) > 0) {
		fwrite(buff, 1, size, dest);
	}
	// Copying the permissions
	struct stat st;
	stat(&source[0], &st);
	chmod(&destination[0], st.st_mode);
	fclose(src);
	fclose(dest);
}

void copyDirectory_roll(char* path, char* des, char* d_name) {
	if (current_directory.find(string(d_name)) != current_directory.end()) {
		remove_dir(&(string(des) + '/' + string(d_name))[0]);
	}

	int status = mkdir(des, S_IRUSR | S_IWUSR | S_IXUSR);
	DIR* d;
	struct dirent* dir;
	d = opendir(path);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if ((string(dir->d_name) == "..") || (string(dir->d_name) == ".")) {
				continue;
			}
			else {
				string finalpath = string(path) + "/" + string(dir->d_name);
				char* newpath = new char[finalpath.length() + 1];
				strcpy(newpath, finalpath.c_str());

				string finaldestpath = string(des) + "/" + string(dir->d_name);
				char* newdestpath = new char[finaldestpath.length() + 1];
				strcpy(newdestpath, finaldestpath.c_str());

				struct stat sb;
				if (stat(newpath, &sb) == -1) {
					perror("lstat");
				}
				else {
					if ((S_ISDIR(sb.st_mode))) {
						copyDirectory_roll(newpath, newdestpath, dir->d_name);
					}
					else {
						copyFile_roll(newpath, newdestpath, dir->d_name);
					}
				}
			}
		}
	}
	else {
		//showError("No such Directory found while copying with path :::::" + string(path));
	}
}

// Copies file from path to destination
void copy_version(char* path, char* des) {
	DIR* d;
	struct dirent* dir;
	d = opendir(path);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if ((string(dir->d_name) == "..") || (string(dir->d_name) == ".")) {
				continue;
			}
			else {
				string finalpath = string(path) + "/" + string(dir->d_name);
				char* newpath = new char[finalpath.length() + 1];
				strcpy(newpath, finalpath.c_str());

				string finaldestpath = string(des) + "/" + string(dir->d_name);
				char* newdestpath = new char[finaldestpath.length() + 1];
				strcpy(newdestpath, finaldestpath.c_str());

				struct stat sb;
				if (stat(newpath, &sb) == -1) {
					perror("lstat");
				}
				else {

					if ((S_ISDIR(sb.st_mode))) {
						copyDirectory_roll(newpath, newdestpath, dir->d_name);
					}
					else {
						copyFile_roll(newpath, newdestpath, dir->d_name);
					}
				}
			}
		}
	}
	else {
		//showError("No such Directory found while copying with path :::::" + string(path));
	}
}

bool check_for_version_delete(string final_source, string path) {
	DIR* d = opendir(final_source.c_str());
	map<string, bool>new_fd;
	if (d) {
		struct dirent* p;
		while ((p = readdir(d)) != NULL) {
			for (auto it : current_directory) {
				if (it == p->d_name) {
					new_fd[p->d_name] == true;
					string s = SHA1::from_file(final_source + "/" + p->d_name);
					string t = SHA1::from_file(path + "/" + p->d_name);
					if (s != t)
						return false;
				}

			}
		}

	}
	return true;
}

bool compareVersions(dirent* a, dirent* b) {
	string s1 = string(a->d_name);
	string s2 = string(b->d_name);
	if (s1.compare(s2) < 0) {
		return true;
	}
	return false;
}

void getDirContents(string path, vector<string>& contents) {
	// Clearing the contents vector
	contents.clear();

	// Opens the directory with cwdPath
	DIR* cwd = opendir(&path[0]);
	struct dirent* files;
	// Reading the contents of the directory
	while ((files = readdir(cwd)) != NULL) {
		string name = files->d_name;
		contents.push_back(name);
	}
	// Closing the directory
	closedir(cwd);
	// Storing the contents in sorted order
	sort(contents.begin(), contents.end());
}

void deleteFile(string path) {
	struct stat fileInfo;
	if (stat(&path[0], &fileInfo) == 0) {
		remove(&path[0]);
	}
}

void deleteFolder(vector<string> files) {
	for (int i = 0; i < files.size(); i++) {
		if (files[i] == "." || files[i] == ".." || files[i] == "git" || files[i] == ".git" || files[i] == "git.cpp")
			continue;
		string path = files[i];
		// Directory delete
		if (isDirExist(path)) {
			// Get all the contents present in the files
			vector<string> dirContents;
			getDirContents(path, dirContents);
			// Making all the names of the retireved folders absolute path
			for (int j = 0; j < dirContents.size(); j++) {
				if (dirContents[j] == "." || dirContents[j] == "..")
					continue;
				dirContents[j] = path + "/" + dirContents[j];
			}
			deleteFolder(dirContents);
			rmdir(&path[0]);
		}
		// File Delete
		else {
			remove(&path[0]);
		}
	}
}

void rollback() {

	string src = ".git/version";
	DIR* d = opendir(src.c_str());

	if (d) {
		struct dirent* p, * q;

		content.clear();

		while ((p = readdir(d)) != NULL) {
			if (strcmp(p->d_name, ".") != 0 && strcmp(p->d_name, "..") != 0) {
				content.push_back(p);
			}
		}
		// sort
		sort(content.begin(), content.end(), compareVersions);

		string src1, final_source;
		for (int i = 0;i < content.size();i++) {
			if (strcmp(content[i]->d_name, ".") != 0 && strcmp(content[i]->d_name, "..") != 0) {
				if (content[i]->d_name > src1)src1 = content[i]->d_name;
			}
		}

		current_directory.clear();

		char path[PATH_MAX];
		getcwd(path, PATH_MAX);
		DIR* e = opendir(path);
		if (e) {
			while ((q = readdir(e)) != NULL) {
				current_directory.insert(q->d_name);
			}
		}

		final_source = ".git/version/" + src1;

		bool check = check_for_version_delete(final_source, path);

		string prev_version, path_prev_version;
		if (check == true) {
			remove_dir(&final_source[0]);

			content.pop_back();

			for (int i = 0;i < content.size();i++) {
				if (strcmp(content[i]->d_name, ".") != 0 && strcmp(content[i]->d_name, "..") != 0) {
					if (content[i]->d_name > prev_version)prev_version = content[i]->d_name;
				}
			}

			path_prev_version = ".git/version/" + prev_version;
			// cout << path_prev_version << endl;
				// Delete files
			char buff[PATH_MAX];
			getcwd(buff, PATH_MAX);
			string cwd = string(buff);
			vector<string> dirContents;
			getDirContents(cwd, dirContents);
			deleteFolder(dirContents);

			copy_version(&path_prev_version[0], path);

			// Find the files in the previous version
			vector<string> previous_version_files;
			chdir(&path_prev_version[0]);
			listFiles(".", previous_version_files);
			string dest = "../../../";
			addFilesToVersionDir(previous_version_files, dest);
			chdir(&cwd[0]);

			// Writing to log
			int countDir = countDirectories("./.git/version");
			time_t rawtime;
			time(&rawtime);
			string text = "Rollback\n";
			text += "version: v_" + to_string(countDir + 1) + " --> v_" + to_string(countDir) + "\n";
			text += "date: " + string(ctime(&rawtime)) + "\n\n";
			updateLog(text);
		}
		else {
			// Files changed 
			// No version folder is deleted
				// Delete files
			char buff[PATH_MAX];
			getcwd(buff, PATH_MAX);
			string cwd = string(buff);
			vector<string> dirContents;
			getDirContents(cwd, dirContents);
			deleteFolder(dirContents);

			copy_version(&final_source[0], path);
			// Deleting the extra files
			// Find the files in the previous version
			vector<string> previous_version_files;
			chdir(&final_source[0]);
			listFiles(".", previous_version_files);
			string dest = "../../../";
			addFilesToVersionDir(previous_version_files, dest);
			chdir(&cwd[0]);

			// Writing to log
			int countDir = countDirectories("./.git/version");
			time_t rawtime;
			time(&rawtime);
			string text = "Rollback\n";
			text += "Rolled back to latest commit\n";
			text += "date: " + string(ctime(&rawtime)) + "\n\n";
			updateLog(text);

		}

	}
}

void retrieve(int version) {
	// Checking the existance of the input version 
	int existing_versions = countDirectories("./.git/version");
	if (version > existing_versions) {
		cout << "Specified version doesn't exists\n";
	}
	// listing all files in the input version
	else {
		vector<string> files_in_version;
		string pathStr = "./.git/version/v_" + to_string(version);
		const char* path = pathStr.c_str();
		listFiles(path, files_in_version);
		for (auto& x : files_in_version) {
			// finding the last occurance of '/' in the file path
			int pos = x.find_last_of('/');
			cout << x.substr(pos + 1) << endl;
		}
	}
}

void retriveSHA(int version, string hashValue) {
	// Checking the existance of the input version 
	int existing_versions = countDirectories("./.git/version");
	if (version > existing_versions) {
		cout << "Specified version doesn't exists\n";
	}
	// listing all files in the input version
	else {
		vector<string> files_in_version;
		string pathStr = "./.git/version/v_" + to_string(version);
		const char* path = pathStr.c_str();
		listFiles(path, files_in_version);
		for (auto& x : files_in_version) {
			// finding the last occurance of '/' in the file path
			int pos = x.find_last_of('/');
			// cout<<x.substr(pos+1)<<endl;
			string sha_calculated = SHA1::from_file(x.substr(pos + 1));
			if (sha_calculated == hashValue) {
				cout << x.substr(pos + 1) << endl;
				return;
			}
		}
		cout << "file with given sha value doesn't exists" << endl;
	}
}

void diff() {
	// Get the files in the current folder
	vector<string> allFiles;
	listFiles(".", allFiles);

	int countDir = countDirectories("./.git/version");
	string destination = "./.git/version/v_" + to_string(countDir);
	for (auto it : allFiles) {
		string path = destination + "/" + it;
		struct stat fileInfo;
		// file present in version folder
		cout << endl << "FILE NAME: " << path << endl;
		cout << "##############################################" << endl;
		if (stat(&path[0], &fileInfo) == 0) {
			string mo = "diff -y --suppress-common-lines " + path + " " + it;
			system(&mo[0]);
		}
		cout << "##############################################" << endl << endl;
	}
}

void pull(string directory) {
	string cwd;
	char buff[PATH_MAX];
	getcwd(buff, PATH_MAX);
	cwd = string(buff);

	chdir(&directory[0]);

	// Get the files from the directory and store in the current directory
	vector<string> files;
	listFiles(".", files);

	addFilesToVersionDir(files, cwd);
	chdir(&cwd[0]);

	// Create .git directory
	init();
}

void log() {
	vector<string> text = readFile("./.git/log.txt");
	cout << endl;
	for (auto it : text) {
		cout << it << endl;
	}
}
// =================================================== main() =============================================================

int main(int argc, char* argv[]) {

	if (argc == 1) {
		cout << "Enter valid arguments" << endl;
		return 0;
	}
	string arg = argv[1];

	if (arg == "init") {
		init();
		cout << "Initialized as git repository" << endl;
	}
	else if (arg == "add") {
		if (argc != 3) {
			cout << "Nothing specified, nothing added." << endl;
			cout << "Use . for all the files" << endl;
			return 0;
		}
		add(argv[2]);
		cout << "Work Staged Successfully" << endl;
	}
	else if (arg == "commit") {
		commit();
		cout << "Work Commited SuccessFully" << endl;
	}
	else if (arg == "status") {
		status();
	}
	else if (arg == "push") {
		if (argc != 3) {
			cout << "Enter the valid number of argumenets" << endl;
			return 0;
		}
		push(argv[2]);
	}
	else if (arg == "rollback") {
		rollback();
		cout << "Directory Rolledback SuccessFully" << endl;
	}
	// ./git retrieve -a vno
	else if (arg == "retrieve" && strcmp(argv[2], "-a") == 0) {
		// cout<<argv[2]<<" "<<argv[3]<<endl;
		retrieve(stoi(argv[3]));
	}
	// ./git retrieve SHAvalue vno
	else if (arg == "retrieve") {
		// cout<<"retrieve SHA is called\n";
		string hashValue = argv[2];
		retriveSHA(stoi(argv[3]), hashValue);
	}
	else if (arg == "diff") {
		diff();
	}
	else if (arg == "pull") {
		if (argc != 3) {
			cout << "Enter the valid number of argumenets" << endl;
			return 0;
		}
		pull(argv[2]);
	}
	else if (arg == "log") {
		log();
	}
	else {
		cout << "Enter valid git command" << endl;
	}
	return 0;
}

