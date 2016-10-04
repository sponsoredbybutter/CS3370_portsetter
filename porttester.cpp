#include <iostream>
#include <fstream>
#include <vector>
#include <string>
using namespace std;

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

vector<string> ps_messages;
string language_code;

bool test_helper(string cmd_str, int expected_code, string success_type = "h", bool include_usage = true, string lang_reject = "", string lang_str = ""){
    bool return_bool = true;
    bool print_okay = true;
    int status_code;
    int diff_code;
    
    cout << "Testing " << cmd_str << " ... ";
    // system takes a const char*, so the dynamic string argument must pass through c_str() first
    status_code = system((cmd_str+" >test.txt").c_str()) / 256;
    if(status_code != expected_code){
        return_bool = false;
        cout << "failed return code";
    }
    
    // Create expected output instead of doing file diffs
    ifstream testfile;
    testfile.open("test.txt", ios::in);
    ifstream compfile;
    string test_in;
    string comp_in;
    
    // Test the default language quasi-error
    if(lang_reject != ""){
        string lang_rej_comp;
        if(lang_reject == "miss") lang_rej_comp = "Missing " + lang_str + " translation files. Using English.";
        if(lang_reject == "err") lang_rej_comp = "Bad language specification in environment variable LANGUAGE=" + lang_str + ". Using English.";
        getline(testfile, test_in);
        if(test_in != lang_rej_comp) print_okay = false;
    }
    
    // check print_okay. Don't need to compare more if it has already failed
    // "h" just needs the usage, that gets compared later
    if(print_okay && expected_code == SUCCESS && success_type != "h"){
        // Test version
        if(success_type == "v") {
            getline(testfile, test_in);
            if(test_in != "1.03") print_okay = false;
            getline(testfile, test_in);
            if(!testfile.eof()) print_okay = false;
       
       // test About
        } else if(success_type == "a") {
            compfile.open("language_files/portsetter.about_"+language_code+".txt", ios::in);
            while(testfile.good()){
                getline(testfile, test_in);
                getline(compfile, comp_in);
                if(test_in != comp_in) {
                    print_okay = false;
                    break;
                }
            }
            compfile.close();
            
        // Test port
        } else {
            // success_type is the port number
            comp_in = ps_messages[SUCCESS] + success_type;
            getline(testfile, test_in);
            if(test_in != comp_in) print_okay = false;
            getline(testfile, test_in);
            if(!testfile.eof()) print_okay = false;
        }
        
    // This is for comparing errors
    } else if(print_okay && expected_code != SUCCESS) {
        getline(testfile, test_in);
        if(test_in != ps_messages[expected_code]) print_okay = false;
        // There should be a blank line
        //getline(testfile, test_in);
    }
    
    if(print_okay && include_usage) {
        compfile.open("language_files/portsetter.usage_"+language_code+".txt", ios::in);
        while(testfile.good()){
            getline(testfile, test_in);
            getline(compfile, comp_in);
            if(test_in != comp_in) {
                print_okay = false;
                break;
            }
        }
    }
    
    testfile.close();
    compfile.close();
    
    if(!print_okay) {
        if(!return_bool) cout << ", ";
        return_bool = false;
        cout << "failed expected printed output";
    }
    
    if(return_bool) cout << "passed";
    cout << endl;
    return return_bool;
}

// Stolen from portsetter.cpp; makes the array of error messages and such for comparison
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

int main(){
    //Setup language and messages
    setenv("LANGUAGE", "en", 1);
    language_code = "en";
    setup_messages();
    
    int return_code = 0;
    
    cout << "Automated portsetter testing" << endl << endl;
    
    // Happy tests
    if(!test_helper("setport", SUCCESS)) return_code = 1;
    if(!test_helper("setport -h", SUCCESS)) return_code = 1;
    if(!test_helper("setport --help", SUCCESS)) return_code = 1;
    if(!test_helper("setport -p 4040", SUCCESS, "4040", false)) return_code = 1;
    if(!test_helper("setport --port 4040", SUCCESS, "4040", false)) return_code = 1;
    
    // Version 1.02, more happy tests
    if(!test_helper("setport -?", SUCCESS)) return_code = 1;
    if(!test_helper("setport -!", SUCCESS, "a", false)) return_code = 1;
    if(!test_helper("setport --about", SUCCESS, "a", false)) return_code = 1;
    if(!test_helper("setport -v", SUCCESS, "v", false)) return_code = 1;
    if(!test_helper("setport --version", SUCCESS, "v", false)) return_code = 1;
    // Set up some environmental variables
    char* current_port = getenv("PORT");
    setenv("PORT", "8080", 1);
    setenv("WUBALUBADUBDUB", "4920", 1);
    if(!test_helper("setport --port -e", SUCCESS, "8080", false)) return_code = 1;
    if(!test_helper("setport -p -e", SUCCESS, "8080", false)) return_code = 1;
    if(!test_helper("setport --port -e WUBALUBADUBDUB", SUCCESS, "4920", false)) return_code = 1;
    if(!test_helper("setport -p -e WUBALUBADUBDUB", SUCCESS, "4920", false)) return_code = 1;
    // Unset some environmental variables
    unsetenv("WUBALUBADUBDUB");
    setenv("PORT", current_port, 1);
    
    // Error codes:
    // 1: flag not recognized
    // 2: Too many arguments used
    // 3: no port number given
    // 4: Environmental variable not found
    // 5: invalid port given
    // 6: Environmental variable not valid
    
    // Sad tests
    if(!test_helper("setport help", UNKNOWN_FLAG)) return_code = 1;
    if(!test_helper("setport -help", UNKNOWN_FLAG)) return_code = 1;
    if(!test_helper("setport --h", UNKNOWN_FLAG)) return_code = 1;
    if(!test_helper("setport -h --help", TOO_MANY_ARGUMENTS)) return_code = 1;
    if(!test_helper("setport -hs", UNKNOWN_FLAG)) return_code = 1;
    if(!test_helper("setport -p --port 9", TOO_MANY_ARGUMENTS)) return_code = 1;
    if(!test_helper("setport -p 77 33", TOO_MANY_ARGUMENTS)) return_code = 1;
    if(!test_helper("setport -p -21", INVALID_PORT)) return_code = 1;
    if(!test_helper("setport -p 0", INVALID_PORT)) return_code = 1;
    if(!test_helper("setport --port", NO_PORT_GIVEN)) return_code = 1;
    if(!test_helper("setport -p 90642", INVALID_PORT)) return_code = 1;
    if(!test_helper("setport -x 45321", UNKNOWN_FLAG)) return_code = 1;
    if(!test_helper("setport -P 714", UNKNOWN_FLAG)) return_code = 1;
    
    // Version 1.02, new tests!
    if(!test_helper("setport -? -h", TOO_MANY_ARGUMENTS)) return_code = 1;
    if(!test_helper("setport --?", UNKNOWN_FLAG)) return_code = 1;
    if(!test_helper("setport -! --about", TOO_MANY_ARGUMENTS)) return_code = 1;
    if(!test_helper("setport --!", UNKNOWN_FLAG)) return_code = 1;
    if(!test_helper("setport -about", UNKNOWN_FLAG)) return_code = 1;
    if(!test_helper("setport --ABOUT", UNKNOWN_FLAG)) return_code = 1;
    if(!test_helper("setport -V", UNKNOWN_FLAG)) return_code = 1;
    if(!test_helper("setport -version", UNKNOWN_FLAG)) return_code = 1;
    if(!test_helper("setport -v --version", TOO_MANY_ARGUMENTS)) return_code = 1;
    if(!test_helper("setport --?", UNKNOWN_FLAG)) return_code = 1;
    // ENVIRONMENTAL VARIABLES
    current_port = getenv("PORT");
    setenv("PORT", "90000", 1);
    if(!test_helper("setport -p -e", ENV_VAR_NOT_VALID)) return_code = 1;
    if(!test_helper("setport --port -e", ENV_VAR_NOT_VALID)) return_code = 1;
    unsetenv("PORT");
    if(!test_helper("setport -p -e", ENV_VAR_NOT_FOUND)) return_code = 1;
    if(!test_helper("setport --port -e", ENV_VAR_NOT_FOUND)) return_code = 1;
    setenv("PORT", current_port, 1);
    if(!test_helper("setport -p -e WUBALUBADUBDUB", ENV_VAR_NOT_FOUND)) return_code = 1;
    if(!test_helper("setport --port -e WUBALUBADUBDUB", ENV_VAR_NOT_FOUND)) return_code = 1;
    setenv("WUBALUBADUBDUB", "125302", 1);
    if(!test_helper("setport -p -e WUBALUBADUBDUB 15", TOO_MANY_ARGUMENTS)) return_code = 1;
    if(!test_helper("setport -p -e WUBALUBADUBDUB", ENV_VAR_NOT_VALID)) return_code = 1;
    if(!test_helper("setport --port -e WUBALUBADUBDUB", ENV_VAR_NOT_VALID)) return_code = 1;
    setenv("WUBALUBADUBDUB", "ants", 1);
    if(!test_helper("setport -p -e WUBALUBADUBDUB", ENV_VAR_NOT_VALID)) return_code = 1;
    if(!test_helper("setport --port -e WUBALUBADUBDUB", ENV_VAR_NOT_VALID)) return_code = 1;
    if(!test_helper("setport -p -E WUBALUBADUBDUB", TOO_MANY_ARGUMENTS)) return_code = 1;
    if(!test_helper("setport --port -E", INVALID_PORT)) return_code = 1;
    unsetenv("WUBALUBADUBDUB");
    
    // VERSION 1.03: SPANISH TESTS
    cout << "Some same tests from before, but setting the language to spanish.\n";
    setenv("LANGUAGE", "es", 1);
    language_code = "es";
    ps_messages.clear();
    setup_messages();
    
    if(!test_helper("setport --help", SUCCESS)) return_code = 1;
    if(!test_helper("setport -p 4040", SUCCESS, "4040", false)) return_code = 1;
    if(!test_helper("setport --about", SUCCESS, "a", false)) return_code = 1;
    if(!test_helper("setport -v", SUCCESS, "v", false)) return_code = 1;
    if(!test_helper("setport --h", UNKNOWN_FLAG)) return_code = 1;
    if(!test_helper("setport -h --help", TOO_MANY_ARGUMENTS)) return_code = 1;
    if(!test_helper("setport -p 0", INVALID_PORT)) return_code = 1;
    if(!test_helper("setport --port", NO_PORT_GIVEN)) return_code = 1;
    setenv("WUBALUBADUBDUB", "ants", 1);
    if(!test_helper("setport -p -e WUBALUBADUBDUB", ENV_VAR_NOT_VALID)) return_code = 1;
    if(!test_helper("setport --port -e BLACKBERRIES", ENV_VAR_NOT_FOUND)) return_code = 1;
    
    cout << "Testing the regex\n";
    setenv("LANGUAGE", "es_PN", 1);
    if(!test_helper("setport -p 4040", SUCCESS, "4040", false)) return_code = 1;
    if(!test_helper("setport --about", SUCCESS, "a", false)) return_code = 1;
    if(!test_helper("setport -p 0", INVALID_PORT)) return_code = 1;
    setenv("LANGUAGE", "es_PN.UTF-8", 1);
    if(!test_helper("setport -p 4040", SUCCESS, "4040", false)) return_code = 1;
    if(!test_helper("setport --about", SUCCESS, "a", false)) return_code = 1;
    if(!test_helper("setport -p 0", INVALID_PORT)) return_code = 1;
    setenv("LANGUAGE", "es.UTF-8", 1);
    if(!test_helper("setport -p 4040", SUCCESS, "4040", false)) return_code = 1;
    if(!test_helper("setport --about", SUCCESS, "a", false)) return_code = 1;
    if(!test_helper("setport -p 0", INVALID_PORT)) return_code = 1;
    
    setenv("LANGUAGE", "QZ.UTF-8", 1);
    language_code = "en";
    ps_messages.clear();
    setup_messages();
    if(!test_helper("setport -p 4040", SUCCESS, "4040", false, "err", "QZ.UTF-8")) return_code = 1;
    if(!test_helper("setport -p 0", INVALID_PORT, "", true, "err", "QZ.UTF-8")) return_code = 1;
    setenv("LANGUAGE", "En", 1);
    if(!test_helper("setport -p 4040", SUCCESS, "4040", false, "err", "En")) return_code = 1;
    if(!test_helper("setport -p 0", INVALID_PORT, "", true, "err", "En")) return_code = 1;
    setenv("LANGUAGE", "ENGL", 1);
    if(!test_helper("setport -p 4040", SUCCESS, "4040", false, "err", "ENGL")) return_code = 1;
    if(!test_helper("setport -p 0", INVALID_PORT, "", true, "err", "ENGL")) return_code = 1;
    setenv("LANGUAGE", "fresh", 1);
    if(!test_helper("setport -p 4040", SUCCESS, "4040", false, "err", "fresh")) return_code = 1;
    if(!test_helper("setport -p 0", INVALID_PORT, "", true, "err", "fresh")) return_code = 1;
    setenv("LANGUAGE", "fr", 1);
    // valid syntax, so it will try to use it
    if(!test_helper("setport -p 4040", SUCCESS, "4040", false, "miss", "fr")) return_code = 1;
    if(!test_helper("setport -p 0", INVALID_PORT, "", true, "miss", "fr")) return_code = 1;
    setenv("LANGUAGE", "de_DE", 1);
    if(!test_helper("setport -p 4040", SUCCESS, "4040", false, "miss", "de")) return_code = 1;
    if(!test_helper("setport -p 0", INVALID_PORT, "", true, "miss", "de")) return_code = 1;
    setenv("LANGUAGE", "cn.UTF-8", 1);
    if(!test_helper("setport -p 4040", SUCCESS, "4040", false, "miss", "cn")) return_code = 1;
    if(!test_helper("setport -p 0", INVALID_PORT, "", true, "miss", "cn")) return_code = 1;
    
    if(return_code == 0) cout << "All Portsetter tests passed!" << endl;
    else cout << "Portsetter failed to pass all required tests." << endl;
    return return_code;
}
