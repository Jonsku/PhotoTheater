#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "MainWindow.h"
#include "UserDefaults.h"
#include "CameraThread.h"


/***********************************************************/
/* PhotoTheater                                            */
/*                                                         */
/* Combines  a full camera                                 */
/* application for the N900 using fcamera and a virtual    */
/* paper theater studio.                                   */
/***********************************************************/
CameraThread *cameraThread;

// Handle Ctrl-C with a clean quit
void sigint(int) {
    cameraThread->stop();
}



int main(int argc, char **argv) {
    QApplication app(argc, argv);

    // We're going to be passing around Frames using Qt Signals, so we
    // need to first register the type with Qt.
    qRegisterMetaType<FCam::Frame>("FCam::Frame");
    qRegisterMetaType<FCam::Event>("FCam::Event");

    //load settings
    UserDefaults &userDefaults = UserDefaults::instance();
    userDefaults["rawPath"] = "/home/user/MyDocs/PhotoTheater/";

    // Make a thread that controls the camera
    cameraThread = new CameraThread();

    // Make the main window
    MainWindow *window = new MainWindow(&app,cameraThread);



    qDebug() << "Starting thread...";
    // Launch the camera thread
    cameraThread->start();


   // Make Ctrl-C call app->exit(SIGINT)
   signal(SIGINT, sigint);

   window->showFullScreen();

   int rval = app.exec();

   printf("About to delete camera thread\n");

   delete cameraThread;
   printf("Camera thread deleted\n");
   return rval;
}
