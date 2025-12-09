#include <iostream>
#include <cstdlib>
using namespace std;

void Start() {
    int num1, num2, result;
    string operation;

    cout << "Enter your first number: ";
    cin >> num1;

    cout << "Choose your operation (+,-,*,/,%) : ";
    cin >> operation;

    cout << "Enter your second number: ";
    cin >> num2;

    if (operation == "+") result = num1 + num2;
    else if (operation == "-") result = num1 - num2;
    else if (operation == "*") result = num1 * num2;
    else if (operation == "/") result = num1 / num2;
    else if (operation == "%") result = num1 % num2;
    else {
        cout << "Invalid operation!" << endl;
        return;
    }

    cout << "The answer is: " << result << endl;
}

int main() {
    string restart;

    while (true) {
        system("cls");       
        cout << "Simple Calculator\n";
        Start();

        cout << "\nPress 'y' to calculate again, 'n' to exit: ";
        cin >> restart;

        if (restart == "n") {
            cout << "See you soon!\n";
            break;      
        }
        else if (restart != "y") {
            cout << "Invalid choice\n";
            break;
        }
    }

    return 0;
}
