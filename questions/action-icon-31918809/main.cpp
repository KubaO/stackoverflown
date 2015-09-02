#include <QApplication>
#include <QAction>
#include <QToolBar>
#include <QGridLayout>
#include <QPainter>
#include <QImage>

class Ui : public QWidget {
   QGridLayout m_grid;
   QToolBar m_actionBar;
   QIcon m_silent, m_speaker;
   QAction m_action4;
   bool m_muteActive;
   QPixmap drawText(const char * text, int size = 64) {
      QPixmap pix(size, size);
      QPainter p(&pix);
      p.setFont(QFont("helvetica", size*0.8));
      p.fillRect(pix.rect(), Qt::white);
      p.drawText(pix.rect(), QString::fromUtf8(text));
      return pix;
   }
public:
   Ui() :
      m_grid(this),
      m_silent(drawText("🔇")),
      m_speaker(drawText("🔊")),
      m_action4(tr("Mute"), this),
      m_muteActive(false)
   {
      m_grid.addWidget(&m_actionBar, 0, 0);
      m_actionBar.addAction(&m_action4);
      connect(&m_action4, &QAction::triggered, this, &Ui::muteMessages);
      updateIcon();
   }
   Q_SLOT void muteMessages() {
      m_muteActive = !m_muteActive;
      updateIcon();
   }
   Q_SLOT void updateIcon() {
      m_action4.setIcon(m_muteActive ? m_silent : m_speaker);
   }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   Ui ui;
   ui.show();
   return a.exec();
}
