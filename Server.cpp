#include <iostream>

using namespace std;

//check if a username is present in the database
string lookup(string username) {
    //change this to SQL query later
    if(username == "farrarng") {
        return "qwerty";
    } else {
        cout << "User not present in database. Create account? Y/N\n";
        return "";
    }
}

//send a comment recieved from a user to the chat
void postComment(string comment) {
    //prepend '<username of sender>: ' to 'comment'
    //send 'comment' to all clients
}

int main(int argc, char const *argv[])
{
    string username;
    string password;
    bool found = false;
    while (!found) {
        //this is just a test, not acutal code
        cout << "Enter username: ";
        cin >> username;
        password = lookup(username);
        if(password == "") {
            cout << "Username not found in the database, try again.\n";
        } else {
            found = true;
        }
    }

    cout << "Enter password: ";
    string input;
    cin >> input;
    while (input != password) {
        cout << "Password does not match, try again.\n";
        cout << "Enter password: ";
        cin >> input;
    }

    return 0;
}
