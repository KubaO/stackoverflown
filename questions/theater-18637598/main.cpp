// Declare the input/output streams, including standard streams like std::cin and std::cout.
#include <iostream>
// Declare the std::string class - it's C++, we should not use C strings!
#include <string>

// Instead of using the entire std namespace, we'll only use the things that come up often.
// This saves some typing, but is safe. Otherwise, who knows what name may clash with something
// in the vast std namespace.
using std::cin;
using std::cout;
using std::endl;

int main() {
    bool ok; // Whether the most recent answer to a question is valid
    bool rated; // Whether the movie is R-rated
    int age; // Customer's age
    int hour; // Hour of the showing
    const int ticketPrice = 5;
    const int discountPrice = ticketPrice * (1.0 - 0.9);
    const int matineePrice = ticketPrice * 0.5;

    // Gather Inputs

    do {
        std::string answer; // Holds the answer to the yes/no question
        cout << "Is the movie R-rated (y/n)? ";
        cin >> answer;
        if (answer.length() > 0) {
            // If the answer is not empty, we can uppercase the first letter.
            // This way we don't have to check for lowercase answers.
            answer[0] = toupper(answer[0]);
        }
        // The answer is valid when it's non-empty and when it begins with either Y/y or N/n
        ok = answer.length() > 0 and (answer[0] == 'Y' or answer [0] == 'N');
        if (not ok) {
            cout << "That's not a valid answer." << endl;
        } else {
            // The answer is valid, so we can set the rated variable.
            rated = answer[0] == 'Y';
        }
    } while (not ok); // Repeat the question while the answer is invalid

    do {
        cout << "How old are you? ";
        cin >> age;
        // The answer is valid when it's between 0 and 150, inclusive.
        ok = age >= 0 and age <= 150;
        if (not ok) {
            cout << "That's not a valid age!" << endl;
        }
    } while (not ok);

    do {
        cout << "What hour do you want the ticket for? ";
        cin >> hour;
        // The hour 0 is mapped to 24.
        if (hour == 0) hour = 24;
        // The answer is valid when it's between 8 and 24, inclusive.
        ok = hour >= 8 and hour <= 24;
        if (not ok) {
            cout << "That's not a valid hour!";
        }
    } while (not ok);

    // Output the Messages

    if (rated and age <= 13) {
        cout << "You must be accompanied by a Janitor" << endl;
    }
    if (age <= 5) {
        cout << "The entrance is free" << endl;
    }
    else if (hour < 18) {
        cout << "Matinee ticket price is " << matineePrice << endl;
    }
    else {
        if (age <= 55) {
            cout << "Ticket price is " << ticketPrice << endl;
        }
        else {
            cout << "You're eligibile for a 10% discount." << endl;
            cout << "Ticket price is " << discountPrice << endl;
        }
    }
}
