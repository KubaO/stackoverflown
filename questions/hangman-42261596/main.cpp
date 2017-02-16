// https://github.com/KubaO/stackoverflown/tree/master/questions/hangman-42261596
#include <QtWidgets>
#include <forward_list>

namespace Ui { // Approximate uic output
struct Hangman {
   QFormLayout * topLayout;
   QHBoxLayout * lettersLayout;
   QLineEdit * letterEntry;
   QPushButton * setup;
   void setupUi(QWidget * widget) {
      topLayout = new QFormLayout{widget};
      topLayout->addRow(lettersLayout = new QHBoxLayout);
      topLayout->addRow("Guess", letterEntry = new QLineEdit);
      letterEntry->setMaxLength(1);
      topLayout->addRow(setup = new QPushButton{"Setup"});
   }
};
}

class Hangman : public QWidget {
   Q_OBJECT
   Ui::Hangman ui;
   QString m_currentWord;
   std::forward_list<QLabel> m_letters;
public:
   explicit Hangman(QWidget * parent = nullptr) :
      QWidget{parent}
   {
      ui.setupUi(this);
      connect(ui.letterEntry, &QLineEdit::returnPressed, [this]{
         auto const text = ui.letterEntry->text();
         if (!text.isEmpty())
            checkInput(text[0]);
      });
      connect(ui.setup, &QPushButton::clicked, [this]{
         setWord(QInputDialog::getText(this, "Setup", "Secret word:",
                                       QLineEdit::Normal, m_currentWord)); // a hack
      });
      setWord("Hello");
   }
   void setWord(const QString & word) {
      m_currentWord = word.toUpper();
      m_letters.clear();
      auto letter = m_letters.before_begin();
      for (int i{}; i < word.size(); ++i) {
         letter = m_letters.emplace_after(letter);
         letter->setFrameShape(QFrame::Box);
         ui.lettersLayout->addWidget(&*letter);
      }
   }
   int checkInput(QChar input) {
      int matched{};
      input = input.toUpper();
      auto letter = m_letters.begin();
      for (int i{}; i < m_currentWord.size(); ++i) {
         if (m_currentWord.at(i) == input) {
            letter->setText(input);
            matched ++;
         }
         letter ++;
      }
      return matched;
   }
};

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   Hangman hangman;
   hangman.show();
   return app.exec();
}
#include "main.moc"

#include <array>

struct Hangman0 {
   QString current_word;
   struct Ui {
      QLabel * label_0, * label_1, * label_2, * label_3, * label_4;
   } * ui = new Ui;
   int check_input(QChar);
};

/// Returns the number of newly matched letters.
int Hangman0::check_input(QChar input) {
   int matched{};
   std::array<QLabel*, 5> visible_word = {ui->label_0, ui->label_1,ui->label_2, ui->label_3,ui->label_4};
   for (int i{}; i < current_word.length(); i++) {
      auto const ch = current_word.at(i);
      if (input == ch && visible_word[i]->text() != ch) {
         visible_word[i]->setText(ch);
         ++ matched;
      }
   }
   return matched;
}
