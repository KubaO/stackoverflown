#include <vector>

enum Suit { SPADES };
enum Value { ACE, TWO };

class Card {
public:
   Card(Suit, Value) {}
};

int main()
{
   std::vector<Card> cards;
   // Correct
   cards.push_back(Card(SPADES, ACE));
   cards.push_back(Card(SPADES, TWO));
   // Wrong
   cards.push_back(Card(Suit::SPADES, Value::ACE));
   cards.push_back(Card(Suite::SPADES, Value::TWO));
   return 0;
}
