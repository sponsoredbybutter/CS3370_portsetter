#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <regex>
#include <vector>
using namespace std;

const float VERSION = 1.04;
const int MAX_PORT = 65535;
string file_path = ""; // Functions all around town will use this; directory of executable
string language_code = "en"; // Language to be used
vector<string> ps_messages; // Messages to be returned depending on return code

// Used as error codes and indicies for messages
enum {
    SUCCESS,
    UNKNOWN_FLAG,
    TOO_MANY_ARGUMENTS,
    NO_PORT_GIVEN,
    ENV_VAR_NOT_FOUND,
    INVALID_PORT,
    ENV_VAR_NOT_VALID
};


// Exception for invalid port given.
// My roommate was very unimpressed by my catching unspecified exceptions for
//  invalid ports, so this will allow me to be more responsible in this regard.
class invalidPortException: public exception {
    virtual const char* what() const throw() {
        return "Given port was not valid.";
    }
};


// Function to determine the file path of the executable
// The point of this is to use language files relative to the executable
// So if everything is put in the same directory and install.sh is used, setport can run from any directory
string find_executable_directory(char* arg0) {
    string exe_dir = arg0; // Convert to string
    return exe_dir.substr(0, exe_dir.find_last_of("/"));
}


// Function to discover the language to use
string setup_language(){
    regex lang_code("^([a-z]{2})(_[A-Z]{2})?(\\.UTF-8)?$"); // acceptable language format: zz, zz_ZZ, zz.UTF-8, zz_ZZ.UTF-8
    regex ignore("^(C|C\\.UTF-8)?$"); // Ignore C, C.UTF-8, and ""
    smatch matches; // The matches for the regex capturing groups
    char* raw_env; // getenv returns a char* ...
    string lang_from_env; // which gets converted to this string
    string possible_vars[] = {"LANGUAGE", "LC_ALL", "LC_MESSAGES", "LANG"}; // The env vars to search, in this order
    
    // Check the possible env variables in order
    for(int i=0; i<4; i++) {
        raw_env = getenv((possible_vars[i]).c_str());
        
        // If getenv returned nullptr, just continue to the next env var. If not, we shall see if it's a valid language code
        if(raw_env != nullptr){
            // Convert to string; strings have nice functions
            lang_from_env = raw_env;
            
            // If the regex matches, then the saved value matched an acceptable format.
            if(regex_match(lang_from_env, matches, lang_code)){
                // See if messages file exists in chosen language
                ifstream file;
                // matches[0] is the whole string, [1] is the first capture group
                string lc = matches[1];
                file.open(file_path+"/language_files/portsetter.messages_"+lc+".txt", ios::in);
                if(file.good()){ 
                    file.close();
                    return lc; 
                }
                file.close();
                cout << "Missing " << lc << " translation files. Using English.\n";
                return "en";
                
            // The env var value did not match the syntax, check to see if it was one of the values that can be ignored (C, C.UTF-8, etc)
            // If it doesn't match one of those, then it's a bizarre and shameful value and must be explicitly rejected
            // If it was a value that can be ignored, just move on to the next env var
            } else if(!regex_match(lang_from_env, matches, ignore)){
                cout << "Bad language specification in environment variable " << possible_vars[i] << "=" << lang_from_env << ". Using English.\n";
                return "en";
            }
        }
    } // End for loop
    
    // If the function hasn't returned at this point, then no acceptable language was found; English it is.
    return "en";
}


// Function to fill the ps_messages array with values from the chosen message file
void setup_messages(){
    ifstream file;
    string in_string;
    file.open(file_path+"/language_files/portsetter.messages_"+language_code+".txt", ios::in);
    while(file.good()){
        getline(file, in_string);
        ps_messages.push_back(in_string);
    }
    file.close();
}


// Common code for reading in and printing a file
// usage() and about() use this
void read_file(string filename){
    ifstream file;
    char in_char;
    file.open(filename, ios::in);
    while(file.get(in_char)){
        cout << in_char;
    }
    file.close();
}

// Function to display the helpful --help stuff
void usage(){
    read_file(file_path+"/language_files/portsetter.usage_"+language_code+".txt");
}

// Function to display the --about message
void about(){
    read_file(file_path+"/language_files/portsetter.about_"+language_code+".txt");
}

// Shortcut to throw an error if too many arguments are given with otherwise valid flag.
// Pass in the number of flags passed, it gets compared to the expected number (Default is 2)
// If they are not equal, an error is thrown
// This won't terminate the program, main will have to do that based upon the output of this function
bool assert_num_arguments(int arg_count, int expected_count = 2){
    if(arg_count > expected_count){
        cout << ps_messages[TOO_MANY_ARGUMENTS] << endl;
        usage();
        return false;
    }
    return true;
}

// Common code for validating port number. Used for -p and -p -e
// Throws an exception if it was invalid, displays listening message if all was well
void use_port(string input_port){
    size_t remnant;
    int port_num;
    try{
        // Try to convert to int
        port_num = stoi(input_port, &remnant);
    } catch(const invalid_argument& ia){
        throw invalidPortException();
    }
    
    // Check if it was actually a number given
    if(remnant != input_port.length()) throw invalidPortException();
    // Check port range
    if(port_num < 1 || port_num > MAX_PORT) throw invalidPortException();
    cout << ps_messages[SUCCESS] << port_num << endl;
}


int main(int argc, char* args[]){
    // Setup file path, language, and messages
    // file_path, language_code, and ps_messages are all declared earlier, for they are used in multiple functions
    file_path = find_executable_directory(args[0]);
    language_code = setup_language();
    setup_messages();
    
    // flag is the arg passed (-p, -h, etc.)
    string flag;
    
    // If no arguments given
    if(argc == 1){
        usage();
        return SUCCESS;
    }
    
    // Convert first argument to string
    flag = args[1];
    
    // HELP flag
    if(flag == "-h" || flag == "--help" || flag == "-?"){
        // Check if the correct number of arguments was given
        if(!assert_num_arguments(argc)) return TOO_MANY_ARGUMENTS;
        usage();
        return SUCCESS;
    }
    
    // PORT flag
    if(flag == "-p" || flag == "--port"){
        // If they did not give a port number or -e; i.e. only 2 arguments passed
        if(argc == 2 ){
            cout << ps_messages[NO_PORT_GIVEN] << endl;
            usage();
            return NO_PORT_GIVEN;
        }
        
        // If -e is used; Convert arg to string first.
        flag = args[2];
        if(flag == "-e" || flag == "--environment"){
            // There should not be more than 4 arguments
            if(!assert_num_arguments(argc, 4)) return TOO_MANY_ARGUMENTS;
            
            // Get the value from the specified env var. Use args[3] if given, else use "PORT"
            char* env_var_value;
            if(argc == 4) env_var_value = getenv(args[3]);
            else env_var_value = getenv("PORT");
            
            // Test environmental variable, see if it was defined. Error out if null or empty
            if(env_var_value == nullptr || env_var_value[0] == '\0'){
                cout << ps_messages[ENV_VAR_NOT_FOUND] << endl;
                usage();
                return ENV_VAR_NOT_FOUND;
            }
            
            // Try to set port with value of environmental variable
            try {
                // env_var_value will be automatically converted into a string here.
                use_port(env_var_value);
                return SUCCESS;
                
            // If that failed, then the port was not valid
            } catch(invalidPortException& e) { 
                cout << ps_messages[ENV_VAR_NOT_VALID] << endl; 
                usage();
                return ENV_VAR_NOT_VALID;
            }
        } // end if -p followed by -e
        
        // Otherwise (if not -e) the next argument should be a port number
        
        // There must not be more than 3 arguments in this case
        if(!assert_num_arguments(argc, 3)) return TOO_MANY_ARGUMENTS;
        
        // Try to set port 
        try {
            // env_var_value should be automatically converted into a string here.
            use_port(args[2]);
            return SUCCESS;
            
        // If that failed, then the port was invalid
        } catch(invalidPortException& e) { 
            cout << ps_messages[INVALID_PORT] << endl;
            usage();
            return INVALID_PORT;
        }
    }
    
    // ABOUT flag
    if(flag == "-!" || flag == "--about"){
        if(!assert_num_arguments(argc)) return TOO_MANY_ARGUMENTS;
        about();
        return SUCCESS;
    }
    
    // VERSION flag
    if(flag == "-v" || flag == "--version"){
        if(!assert_num_arguments(argc)) return TOO_MANY_ARGUMENTS;
        cout << VERSION << endl;
        return SUCCESS;
    }
    
    // Program will have returned at this point if the given flag was recognized,
    // So if it gets to this point, it's an unknown flag
    cout << ps_messages[UNKNOWN_FLAG] << endl;
    usage();
    return UNKNOWN_FLAG;
}
