using namespace std;

void shadeScene(int view_mode){
  if (view_mode == 1) {
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //wireframe
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (view_mode == 2) glShadeModel(GL_FLAT);
    if (view_mode == 3) glShadeModel(GL_SMOOTH);
    if (view_mode == 4) glShadeModel(GL_SMOOTH);

    //GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 }; //black
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 }; //white
    //GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 10.0, 10.0, 10.0, 0.0 };

    glClearColor(0.0, 0.0, 0.0, 0.0);

    //glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    //glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
  }
}
