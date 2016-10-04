#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <regex>
#include <vector>
using namespace std;

const float VERSION = 1.03;
const int MAX_PORT = 65535;
string language_code = "en";
vector<string> ps_messages;

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

// Function to discover the language to use
string setup_language(){
    // acceptable language format: zz, zz_ZZ, zz.UTF-8, zz_ZZ.UTF-8
    regex lang_code("^([a-z]{2})(_[A-Z]{2})?(\\.UTF-8)?$");
    // Ignore C, C.UTF-8, and ""
    regex ignore("^(C|C\\.UTF-8)?$");
    smatch matches;
    char* raw_env;
    string lang_from_env;
    string possible_vars[] = {"LANGUAGE", "LC_ALL", "LC_MESSAGES", "LANG"};
    
    // Check the possible env variables in order
    for(int i=0; i<4; i++) {
        raw_env = getenv("LANGUAGE");
        if(raw_env != nullptr){
            // Convert to string
            lang_from_env = raw_env;
            // If true, then the saved value matched an acceptable format.
            // If false, continue to next env var
            if(regex_match(lang_from_env, matches, lang_code)){
                // See if messages file exists in chosen language
                ifstream file;
                // matches[0] is the whole string, [1] is the first capture group
                string lc = matches[1];
                file.open("language_files/portsetter.messages_"+lc+".txt", ios::in);
                if(file.good()){ 
                    file.close();
                    return lc; 
                }
                file.close();
                cout << "Missing " << lc << " translation files. Using English.\n";
                return "en";
                
            // It doesn't match the syntax, and it's not one of the values we can ignore
            } else if(!regex_match(lang_from_env, matches, ignore)){
                cout << "Bad language specification in environment variable " << possible_vars[i] << "=" << lang_from_env << ". Using English.\n";
                return "en";
            }
        }
    }
    
    return "en";
}

// Function to fill the ps_messages array with values from the chosen message file
void setup_messages(){
    ifstream file;
    string in_string;
    int i = 0;
    file.open("language_files/portsetter.messages_"+language_code+".txt", ios::in);
    while(file.good()){
        getline(file, in_string);
        ps_messages.push_back(in_string);
    }
    file.close();
}

// Common code for reading in and printing a file
void read_file(string filename){
    ifstream file;
    char in_char;
    file.open(filename, ios::in);
    while(file.get(in_char)){
        cout << in_char;
    }
    file.close();
}

void usage(){
    read_file("language_files/portsetter.usage_"+language_code+".txt");
}

void about(){
    read_file("language_files/portsetter.about_"+language_code+".txt");
}

// Shortcut to throw an error if too many arguments are given with otherwise valid flag.
bool assert_num_arguments(int arg_count, int expected_count = 2){
    if(arg_count > expected_count){
        cout << ps_messages[TOO_MANY_ARGUMENTS] << endl;
        usage();
        return false;
    }
    return true;
}

// Common code 
void use_port(string input_port){
    size_t remnant;
    int port_num;
    try{
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
    // Setup language and messages
    language_code = setup_language();
    setup_messages();
    
    string flag;
    
    // No arguments given
    if(argc == 1){
        usage();
        return SUCCESS;
    }
    
    // Convert first argument to string,
    flag = args[1];
    
    // HELP flag
    if(flag == "-h" || flag == "--help" || flag == "-?"){
        if(!assert_num_arguments(argc)) return TOO_MANY_ARGUMENTS;
        usage();
        return SUCCESS;
    }
    
    // PORT flag
    if(flag == "-p" || flag == "--port"){
        // If they did not give a port number or -e
        if(argc == 2 ){
            cout << ps_messages[NO_PORT_GIVEN] << endl;
            usage();
            return NO_PORT_GIVEN;
        }
        
        // If -e is used; Convert arg to string first.
        flag = args[2];
        if(flag == "-e"){
            // There should not be mor than 4 arguments
            if(!assert_num_arguments(argc, 4)) return TOO_MANY_ARGUMENTS;
            
            char* env_var_value;
            if(argc == 4) env_var_value = getenv(args[3]);
            else env_var_value = getenv("PORT");
            
            // Test environmental variable, see if it was defined
            if(env_var_value == nullptr || env_var_value[0] == '\0'){
                cout << ps_messages[ENV_VAR_NOT_FOUND] << endl;
                usage();
                return ENV_VAR_NOT_FOUND;
            }
            
            // Try to set port with value of environmental variable
            try {
                // env_var_value should be automatically converted into a string here.
                use_port(env_var_value);
                return SUCCESS;
            } catch(invalidPortException& e) { 
                cout << ps_messages[ENV_VAR_NOT_VALID] << endl; 
                usage();
                return ENV_VAR_NOT_VALID;
            }
        }
        
        // Otherwise the next argument should be a port number
        
        // There must not be more than 3 arguments in this case
        if(!assert_num_arguments(argc, 3)) return TOO_MANY_ARGUMENTS;
        
        // Try to set port 
        try {
            // env_var_value should be automatically converted into a string here.
            use_port(args[2]);
            return SUCCESS;
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
    
    // Otherwise, it's an unknown flag
    cout << ps_messages[UNKNOWN_FLAG] << endl;
    usage();
    return UNKNOWN_FLAG;
}
