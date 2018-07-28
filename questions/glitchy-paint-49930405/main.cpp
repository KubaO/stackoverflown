// https://github.com/KubaO/stackoverflown/tree/master/questions/glitchy-paint-49930405
#include <QtWidgets>
#include <Qt3DRender>
#include <Qt3DExtras>
#include <QOpenGLFunctions_3_0>
#include <array>

class Render3dModel : public QWidget {
   struct Model {
      mutable int counter = {};
      QSize size;
      QFont font{"Helvetica", 48};
      QImage getImage() const {
         static auto const format = QPixmap{1,1}.toImage().format();
         QImage image{size, format};
         image.fill((counter & 1) ? Qt::blue : Qt::yellow);
         QPainter p(&image);
         p.setFont(font);
         p.setPen((counter & 1) ? Qt::yellow : Qt::blue);
         p.drawText(image.rect(), QString::number(counter));
         counter++;
         return image;
      }
   } m_model;
   void paintEvent(QPaintEvent *) override {
      m_model.size = size();
      auto image = m_model.getImage();
      QPainter p{this};
      p.drawImage(QPoint{0, 0}, image);
      p.
   }
};

int main(int argc, char **argv) {
   QApplication app{argc, argv};
   //Scene scene;
   QWidget win;
   QVBoxLayout topLayout{&win};
   QTabWidget tabs;
   topLayout.addWidget(&tabs);
   // Tabs
   for (auto text : { "Shape", "Dimensions", "Layout"}) tabs.addTab(new QWidget, text);
   tabs.setCurrentIndex(1);
   QHBoxLayout tabLayout{tabs.currentWidget()};
   QGroupBox dims{"Section Dimensions"}, model{"3D Model"};
   QGridLayout dimsLayout{&dims}, modelLayout{&model};
   for (auto w : {&dims, &model}) tabLayout.addWidget(w);
   // Section Dimensions
   for (auto text : {"Diameter 1", "Diameter 2", "Length"}) {
      auto row = dimsLayout.rowCount();
      std::array<QWidget*, 3> widgets{{new QLabel{text}, new QDoubleSpinBox, new QLabel{"inch"}}};
      for (auto *w : widgets)
         dimsLayout.addWidget(w, row, dimsLayout.count() % widgets.size());
   }
   QPushButton update{"Update"};
   dimsLayout.addWidget(&update, dimsLayout.count()/dimsLayout.columnCount(), 0, 1, dimsLayout.columnCount());
   update.setAutoRepeat(true);
   update.setAutoRepeatDelay(0);
   update.setAutoRepeatInterval(1000/50);
   tabLayout.setAlignment(&dims, Qt::AlignLeft | Qt::AlignTop);
   dims.setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
   // Model
   Render3dModel render;
   modelLayout.addWidget(&render);
   model.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
   model.setMinimumSize(250, 250);
   QObject::connect(&update, &QPushButton::clicked, &render, [&]{ render.update(); });
   win.show();
   return app.exec();
}

struct OpenGLOffscreenSurface : QOffscreenSurface {
   QOpenGLContext context;
   QOpenGLFunctions *functions = {};
   QOpenGLFunctions_3_0 *functions_3_0 = {};
   QOpenGLFramebufferObject *fbo = {};
   OpenGLOffscreenSurface() {
      setFormat(QSurfaceFormat::defaultFormat());
      create();
      context.setFormat(format());
      if (context.create()) {
         context.makeCurrent(this);
         functions = context.functions();
         functions->initializeOpenGLFunctions();
         functions_3_0 = context.versionFunctions<QOpenGLFunctions_3_0>();
         if (functions_3_0)
            functions_3_0->initializeOpenGLFunctions();
      }
   }
};

struct Scene {
   Qt3DCore::QAspectEngine      aspectEngine;
   Qt3DRender::QRenderAspect   *renderAspect = new Qt3DRender::QRenderAspect;
   Qt3DRender::QRenderSettings  renderSettings;
   Qt3DExtras::QForwardRenderer renderer;
   Qt3DCore::QEntity  *root = new Qt3DCore::QEntity;
   Qt3DRender::QCamera camera{root};

   Qt3DCore::QEntity       lightE{root};
   Qt3DRender::QPointLight light{&lightE};
   Qt3DCore::QTransform    lightTransform{&lightE};

   Qt3DCore::QEntity          coneE{root};
   Qt3DExtras::QConeMesh      cone{&coneE};
   Qt3DCore::QTransform       coneTransform{&coneE};
   Qt3DExtras::QPhongMaterial coneMaterial{&coneE};

   Qt3DExtras::QFirstPersonCameraController camController{root};
   Scene();
};

Scene::Scene() {
   aspectEngine.registerAspect(renderAspect);
   renderer.setCamera(&camera);
   renderSettings.setActiveFrameGraph(&renderer);

   renderer.setClearColor(Qt::gray);
   camera.lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
   camera.setPosition({0.f, 0.f, 20.0f});
   camera.setUpVector({0.f, 1.f, 0.f});
   camera.setViewCenter({});
   renderer.setCamera(&camera);
   camController.setCamera(&camera);

   light.setColor(Qt::white);
   light.setIntensity(1);
   lightE.addComponent(&light);
   lightTransform.setTranslation(camera.position());
   lightE.addComponent(&lightTransform);

   cone.setTopRadius(0.5f);
   cone.setBottomRadius(1.0f);
   cone.setLength(3.f);
   cone.setRings(50);
   cone.setSlices(20);
   coneE.addComponent(&cone);

   coneTransform.setScale(1.5f);
   coneE.addComponent(&coneTransform);

   coneMaterial.setDiffuse(Qt::darkGray);
   coneE.addComponent(&coneMaterial);

   root->addComponent(&renderSettings);
   aspectEngine.setRootEntity(Qt3DCore::QEntityPtr{root});
}


