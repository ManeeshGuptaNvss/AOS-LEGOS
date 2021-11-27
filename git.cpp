#include<iostream>
#include<fstream>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<string.h>
#include<iterator>
#include<map>
#include<set>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<dirent.h>

#include"sha1.hpp"

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define RESET "\033[0m"

using namespace std;

unordered_map<string, string> statusFiles;
vector<string> newFiles, modifiedFiles, deletedFiles;

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
	int countDir = countDirectories("./.git/version");
	time_t rawtime;
	time(&rawtime);

	FILE* fp = fopen("./.git/log.txt", "a");
	fwrite(&text[0], 1, text.length(), fp);
	fclose(fp);
}

// Copy files into the new version directory
void addFilesToVersionDir(vector<string>& files) {
	// Create the version Directory
	string destination = "./.git/version";
	int countDir = countDirectories(destination);
	destination += +"/v_" + to_string(countDir + 1);
	if (mkdir(&destination[0], 0777) == -1) {
		perror("Version Directory not created");
		return;
	}
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

// Reading files from status.txt
void readStatusFile() {
	//read status.txt file
	ifstream ifs;
	ifs.open("./.git/status.txt");

	while (!ifs.eof()) {
		string line, stData, data;
		int flag;
		if (getline(ifs, line)) {
			vector<string> words = splitString(line);
			statusFiles[words[0]] = words[1];
		}
	}
	ifs.close();
}

// Print status files
void printStatus(vector<string> data, const char* color, unordered_set<string>& addFiles, vector<string>& result) {
	bool flag = true;
	for (auto j : data) {
		if (addFiles.find(j) != addFiles.end()) {
			result.push_back(color + j + RESET);
		}
		else {
			if (flag) {
				cout << "Untracked Files:" << endl;
				flag = false;
			}
			cout << "\t" << color + j + RESET << endl;
		}
	}
}


// Checking which files are newly created, modified and deleted
void checkFiles() {
	vector<string> files;
	listFiles(".", files);
	// Get files from the add.txt file
	unordered_set<string> addFiles;
	ifstream ifs;
	ifs.open("./.git/add.txt");
	while (!ifs.eof()) {
		string line, stData, data;
		int flag;
		if (getline(ifs, line)) {
			addFiles.insert(line);
		}
	}
	ifs.close();

	vector<string> result;
	vector<string> untrackedFiles;
	vector<string> trackedFiles;

	unordered_set<string> us;
	//okay
	for (auto i : files) {
		us.insert(i);
		if (statusFiles.find(i) != statusFiles.end()) {
			string hash = SHA1::from_file(i);
			if (statusFiles[i] != hash) {
				result.push_back(i);
			}
		}
		if (addFiles.find(i) == addFiles.end()) {
			if (statusFiles.find(i) == statusFiles.end())
				untrackedFiles.push_back(i);
		}
		else {
			trackedFiles.push_back(i);
		}
	}
	for (auto i : statusFiles) {
		if (us.find(i.first) == us.end()) {
			result.push_back(i.first + " (deleted)");
		}
	}
	// so far so good
	if (untrackedFiles.size() != 0) {
		cout << "Untracked files:" << endl;
		for (auto i : untrackedFiles) {
			cout << KRED "\t" << i + RESET << endl;
		}
	}
	if (trackedFiles.size() != 0) {
		cout << "Changes to be committed:" << endl;
		for (auto i : trackedFiles) {
			cout << KGRN "\t" << i + RESET << endl;
		}
	}
	if (result.size() != 0) {
		cout << "Modified files:" << endl;
		for (auto i : result) {
			cout << KYEL "\t" << i + RESET << endl;
		}
	}
	if (untrackedFiles.size() == 0 && trackedFiles.size() == 0 && result.size() == 0) {
		cout << "Working directory clean" << endl;
	}
}

// ================================================ commands ==========================================================

// Initializes git repo
// TODO: What if the directory is already as repo
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
	vector<string> files;
	vector<string> newFiles;
	readStatusFile();
	if (file == ".") {
		listFiles(".", files);
		for (auto i : files) {
			string hash = SHA1::from_file(i);
			if (statusFiles.find(i) == statusFiles.end() || statusFiles[i] != hash) {
				newFiles.push_back(i);
			}
		}
	}
	else {
		files.push_back(file);
	}
	string dest = "./.git/add.txt";
	writeData(newFiles, dest);
	updateStatus(files);
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

	// 2. Create a new version folder and add the files in it
	vector<string> files = readFile("./.git/add.txt");
	addFilesToVersionDir(files);

	readStatusFile();
	for (auto i : files) {
		// i is a new file or modified version of a file in the previous version
		// if old version exists, remove it
		string hash = SHA1::from_file(i);
		if (statusFiles.find(i) != statusFiles.end()) {
			statusFiles.erase(i);
		}
	}

	// now update files list
	for (auto i : statusFiles) {
		files.push_back(i.first);
	}
	updateStatus(files);

	// 4. Delete the data in add.txt
	string file = "./.git/add.txt";
	deleteData(file);
}

void status() {
	readStatusFile();
	checkFiles();
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

//------------------------Do Not Delete any Function even if it is repeated because they have minor changes----------RollBack---



set<string>current_directory;
vector<dirent*> content;


void SplitString2(string s, vector<string> &v){
	
	string temp = "";
	for(int i=0;i<s.length();++i){
		
		if(s[i]=='/'){
			v.push_back(temp);
			temp = "";
		}
		else{
			temp.push_back(s[i]);
		}
		
	}
	v.push_back(temp);
	
}



void delete_file(string Destination ){
	unlink(Destination.c_str());
	return;



}


int remove_dir(const char *path) {
   DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (d) {
      struct dirent *p;

      r = 0;
      while (!r && (p=readdir(d))) {
          int r2 = -1;
          char *buf;
          size_t len;

          /* Skip the names "." and ".." as we don't want to recurse on them. */
          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
             continue;

          len = path_len + strlen(p->d_name) + 2; 
          buf =(char *) malloc(len);

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



void copyFile_roll(string source, string destination, char * d_name) {
	// cout << source << " " << destination << endl;
	
	
	
	if(current_directory.find(string(d_name))!=current_directory.end()){
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
	if(current_directory.find(string(d_name))!=current_directory.end()){
		remove_dir(&(string(des)+'/'+string(d_name))[0]);
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
						copyDirectory_roll(newpath, newdestpath,dir->d_name);
					}
					else {
						copyFile_roll(newpath, newdestpath,dir->d_name);
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
						copyFile_roll(newpath, newdestpath,dir->d_name);
					}
				}
			}
		}
	}
	else {
		//showError("No such Directory found while copying with path :::::" + string(path));
	}
}

bool check_for_version_delete(string final_source, string path){

	DIR* d = opendir(final_source.c_str());
	map<string,bool>new_fd;
	if(d){
		struct dirent* p;
		while ((p = readdir(d)) != NULL) {
 			for(auto it:current_directory){
 				if(it==p->d_name){
 					new_fd[p->d_name]==true;
 					SHA1 s,t;
 					if(s.from_file(final_source+"/"+p->d_name)!=t.from_file(path+"/"+p->d_name))return false;
 				}
 				
 			}
 		}
	
	}
	return true;

}


void rollback(){

	string src = ".git/version";
	DIR* d = opendir(src.c_str());
	
	if (d) {
 		struct dirent* p,*q;
		
		content.clear();
		
 		while ((p = readdir(d)) != NULL) {
 			content.push_back(p);
 		}
 		
 		string src1, final_source;
 		for (int i = 0;i < content.size();i++) {
 			if (content[i]->d_name != "." && content[i]->d_name != "..") {
 				if (content[i]->d_name > src1)src1 = content[i]->d_name;
 			}
 		}
 		
 		current_directory.clear();
 		
 		char path[PATH_MAX];
 		getcwd(path, PATH_MAX);
 		DIR* e = opendir(path);
 		if(e){
 			while ((q = readdir(e)) != NULL) {
 				current_directory.insert(q->d_name);
 			}
 		}
 		
 		final_source = ".git/version/" + src1;
 		
 		bool check = check_for_version_delete(final_source,path);
 		
 		string prev_version, path_prev_version=final_source;
 		if(check==true){
	 		remove_dir(&final_source[0]);
	 		
	 		content.pop_back();
	 		
	 		for (int i = 0;i < content.size();i++) {
	 			if (content[i]->d_name != "." && content[i]->d_name != "..") {
	 				if (content[i]->d_name > prev_version)prev_version = content[i]->d_name;
	 			}
	 		}
	 		path_prev_version = ".git/version/" + prev_version;
	 		//cout<<prev_version<<endl;
 		}
 		
 		
 		
 		copy_version(&path_prev_version[0],path);
 		
 		
 		
 		
 		
 		
 	
 	}
 		



}

void retrieve(int version)
{
	// Checking the existance of the input version 
	int existing_versions=countDirectories("./.git/version");
	if(version>existing_versions)
	{
		cout<<"Specified version doesn't exists\n";
	}
	// listing all files in the input version
	else{
		vector<string> files_in_version;
		string pathStr="./.git/version/v_"+to_string(version);
		const char* path = pathStr.c_str();
		listFiles(path,files_in_version);
		for(auto &x:files_in_version)
		{	
			// finding the last occurance of '/' in the file path
			int pos=x.find_last_of('/');
			cout<<x.substr(pos+1)<<endl;
		}
	}
}

void retriveSHA(int version,string hashValue)
{
	// Checking the existance of the input version 
	int existing_versions=countDirectories("./.git/version");
	if(version>existing_versions)
	{
		cout<<"Specified version doesn't exists\n";
	}
	// listing all files in the input version
	else{
		vector<string> files_in_version;
		string pathStr="./.git/version/v_"+to_string(version);
		const char* path = pathStr.c_str();
		listFiles(path,files_in_version);
		for(auto &x:files_in_version)
		{	
			// finding the last occurance of '/' in the file path
			int pos=x.find_last_of('/');
			// cout<<x.substr(pos+1)<<endl;
			string sha_calculated=SHA1::from_file(x.substr(pos+1));
			if(sha_calculated==hashValue)
			{
				cout<<x.substr(pos+1)<<endl;
				return;
			}
		}
		cout<<"file with given sha value doesn't exists"<<endl;
	}

}



// ====================================================================================================================
int main(int argc, char* argv[]) {

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
	}
	else if (arg == "commit") {
		commit();
		cout<<"Work Commited SuccessFully"<<endl;
	}
	else if (arg == "status") {
		status();
	}
	else if (arg == "push") {
		if (argc != 3) {
			cout << "[ERR]Enter the valid number of argumenets" << endl;
			return 0;
		}
		push(argv[2]);
	}
	else if(arg=="rollback"){
		rollback();
		cout<<"Directory Rolledback SuccessFully"<<endl;
	}
	// ./git retrieve -a vno
	else if (arg=="retrieve" && strcmp(argv[2],"-a")==0 )
	{
		// cout<<argv[2]<<" "<<argv[3]<<endl;
		retrieve(stoi(argv[3]));

	}
	// ./git retrieve SHAvalue vno
	else if(arg=="retrieve" )
	{
		// cout<<"retrieve SHA is called\n";
		string hashValue=argv[2];
		retriveSHA(stoi(argv[3]),hashValue);
	}
	return 0;
}
