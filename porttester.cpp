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
string file_path = ""; // Functions all around town will use this; directory of executable
string language_code;


// test_helper():
// Function to test a given use case of setport
// It will save the output of setport to a test file, then compare it against the expected output
// The expected output is created somewhat dynamically from the usage/about/messages files, because
//   system diff was no longer a good option once Spanish moved in.
// Parameters:
//  - cmd_str: the setport command to pass to system; e.g. setport -p 60
//  - expected_code: the expected return code for the given command.
// NOTE: those two parameters are required. If testing an expected error, no other arguments are necessary
//
//  - success_type: h (default) (for help), v (for version), a (for about), or port number
//          Because there are various ways to get return code 0, and the output depends on input
//  - include_usage: include usage() if true (default), don't include if false.
//          Because -p and -a and -v don't need to include the usage, but most cases do include it
//  - lang_reject: miss or err or empty (default). For dealing with an initial language error.
//          If empty (""), this will be ignored, which is the usual case.
//          miss is if the env var produced a valid language (valid syntax, anyways), but the language pack is missing
//          err is if the env var produced an unusable value.
//          lang_reject must use lang_str
//  - lang_str: The actual language code to be used in the language error.

bool test_helper(string cmd_str, int expected_code, string success_type = "h", bool include_usage = true, string lang_reject = "", string lang_str = ""){
    bool return_bool = true; // value to return; true if it the command matched the expected output, false otherwise.
    bool print_okay = true; // intermediary bool to check the printed output specifically, so it can display two separate errors (for status code and printed output)
    int status_code; // status code that the setport command returns
    
    cout << "Testing " << cmd_str << " ... ";
    
    // Run the setport command, save output to test.txt
    // system takes a const char*, so the combined string argument must pass through c_str() first
    status_code = system((cmd_str+" >test.txt").c_str()) / 256;
    // If the return code was not what was expected, display error
    if(status_code != expected_code){
        return_bool = false;
        cout << "failed return code";
    }
    
    // Compare test output to expected output
    // Create expected output within function instead of doing file diffs
    ifstream testfile; // test.txt file
    testfile.open("test.txt", ios::in);
    ifstream compfile; // file to compare against; e.g. portsetter.usage_en.txt, etc
    string test_in; // Line read in from test.txt
    string comp_in; // Line read in from comparison file
    
    // Test the default language quasi-error
    if(lang_reject != ""){
        string lang_rej_comp; // error to compare against
        if(lang_reject == "miss") lang_rej_comp = "Missing " + lang_str + " translation files. Using English.";
        if(lang_reject == "err") lang_rej_comp = "Bad language specification in environment variable LANGUAGE=" + lang_str + ". Using English.";
        getline(testfile, test_in);
        // So if the first line of the test file doesn't match that, then it fails the printed output
        if(test_in != lang_rej_comp) print_okay = false;
    }
    
    // Check print_okay. If it is already false, then there is no need to do more comparisons
    // Check the various success cases
    // Except for "h". Help just needs the usage, that gets compared later (since most cases use usage())
    if(print_okay && expected_code == SUCCESS && success_type != "h"){
        
        // Test Version
        if(success_type == "v") {
            getline(testfile, test_in);
            // Some might argue it is ghetto to just hardcode the version
            // But here it is; such is life
            if(test_in != "1.04") print_okay = false;
            
            // There should be no more output after this. So if we're not at the end of the test.txt file, fail.
            getline(testfile, test_in);
            if(!testfile.eof()) print_okay = false;
       
       // test About
        } else if(success_type == "a") {
            compfile.open(file_path+"/language_files/portsetter.about_"+language_code+".txt", ios::in);
            // So just run through the whole about file and see if it matches the output
            while(testfile.good()){
                getline(testfile, test_in);
                getline(compfile, comp_in);
                if(test_in != comp_in) {
                    print_okay = false;
                    break;
                }
            }
            compfile.close();
            
        // Test port - if success_type isn't a, h, or v, then it should be a port number.
        } else {
            // So the string to compare is Listening on port [port]
            comp_in = ps_messages[SUCCESS] + success_type;
            getline(testfile, test_in);
            if(test_in != comp_in) print_okay = false;
            getline(testfile, test_in);
            if(!testfile.eof()) print_okay = false;
        }
        
    // Check if the expected code is not 0, i.e. it's testing an error
    } else if(print_okay && expected_code != SUCCESS) {
        getline(testfile, test_in);
        if(test_in != ps_messages[expected_code]) print_okay = false;
    }
    
    // Check to see if it printed the usage properly.
    // This applies to errors and --help
    if(print_okay && include_usage) {
        compfile.open(file_path+"/language_files/portsetter.usage_"+language_code+".txt", ios::in);
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
    
    // If the printed output failed, display error
    if(!print_okay) {
        if(!return_bool) cout << ", ";
        return_bool = false;
        cout << "failed expected printed output";
    }
    
    if(return_bool) cout << "passed";
    cout << endl;
    return return_bool;
}

// Function to determine the file path of the executable
// The point of this is to use language files relative to the executable
// So if everything is put in the same directory and install.sh is used, setport can run from any directory
string find_executable_directory(char* arg0) {
    string exe_dir = arg0; // Convert to string
    return exe_dir.substr(0, exe_dir.find_last_of("/"));
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

int main(int argc, char* args[]){
    //Setup file path, language, and messages
    file_path = find_executable_directory(args[0]);
    setenv("LANGUAGE", "en", 1); // setenv determines what setport will use in the tests
    language_code = "en"; // language_code is for this test program; doesn't affect output of setport
    setup_messages(); // sets up this test program's vector of messages; also doesn't affect setport
    
    // It's going to run all the tests before returning, and the whole overall test will fail
    //  if any of the tests fail. So there is a return code instead of just returning immediately
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
