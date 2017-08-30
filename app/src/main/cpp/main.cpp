#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

// Do we want to showcase direct JNI/NDK interaction?
// Undefine this to get real cross-platform code.
#define USE_JNI

#if defined(USE_JNI)
// These headers are only needed for direct NDK/JDK interaction
#include <jni.h>
#include <android/native_activity.h>

// Since we want to get the native activity from SFML, we'll have to use an
// extra header here:
#include <SFML/System/NativeActivity.hpp>

#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

// NDK/JNI sub example - call Java code from native code
int vibrate(sf::Time duration)
{
  // First we'll need the native activity handle
  ANativeActivity *activity = sf::getNativeActivity();

  // Retrieve the JVM and JNI environment
  JavaVM* vm = activity->vm;
  JNIEnv* env = activity->env;

  // First, attach this thread to the main thread
  JavaVMAttachArgs attachargs;
  attachargs.version = JNI_VERSION_1_6;
  attachargs.name = "NativeThread";
  attachargs.group = NULL;
  jint res = vm->AttachCurrentThread(&env, &attachargs);

  if (res == JNI_ERR)
    return EXIT_FAILURE;

  // Retrieve class information
  jclass natact = env->FindClass("android/app/NativeActivity");
  jclass context = env->FindClass("android/content/Context");

  // Get the value of a constant
  jfieldID fid = env->GetStaticFieldID(context, "VIBRATOR_SERVICE", "Ljava/lang/String;");
  jobject svcstr = env->GetStaticObjectField(context, fid);

  // Get the method 'getSystemService' and call it
  jmethodID getss = env->GetMethodID(natact, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
  jobject vib_obj = env->CallObjectMethod(activity->clazz, getss, svcstr);

  // Get the object's class and retrieve the member name
  jclass vib_cls = env->GetObjectClass(vib_obj);
  jmethodID vibrate = env->GetMethodID(vib_cls, "vibrate", "(J)V");

  // Determine the timeframe
  jlong length = duration.asMilliseconds();

  // Bzzz!
  env->CallVoidMethod(vib_obj, vibrate, length);

  // Free references
  env->DeleteLocalRef(vib_obj);
  env->DeleteLocalRef(vib_cls);
  env->DeleteLocalRef(svcstr);
  env->DeleteLocalRef(context);
  env->DeleteLocalRef(natact);

  // Detach thread again
  vm->DetachCurrentThread();

    return EXIT_SUCCESS;
}

int testCall(std::string arg)
{

    // First we'll need the native activity handle
    ANativeActivity *activity = sf::getNativeActivity();

    // Retrieve the JVM and JNI environment
    JavaVM* vm = activity->vm;
    JNIEnv* env = activity->env;

    // First, attach this thread to the main thread
    JavaVMAttachArgs attachargs;
    attachargs.version = JNI_VERSION_1_6;
    attachargs.name = "main";
    attachargs.group = NULL;
    jint res = vm->AttachCurrentThread(&env, &attachargs);

    if (res == JNI_ERR)
        return EXIT_FAILURE;

    jclass apiclass = env->GetObjectClass(activity->clazz);

    if (apiclass != nullptr)
    {
        jmethodID methodID = env->GetMethodID(apiclass, "testCall", "(Ljava/lang/String;)V");

        jstring jstr123 = env->NewStringUTF(arg.c_str());

        env->CallVoidMethod(activity->clazz, methodID, jstr123);

        // Free references
        env->DeleteLocalRef(jstr123);
    } else {
        LOGW("NOPE!");
    }

    vm->DetachCurrentThread();


    return EXIT_SUCCESS;
}

extern "C"
{
JNIEXPORT jstring JNICALL
Java_de_flatspotsoftware_testapp_CPPCallIns_testCall(JNIEnv *env, jobject thiz, jstring arg) {
    std::string str = std::string(env->GetStringUTFChars(arg, JNI_FALSE));
    str.append(str);
    return env->NewStringUTF(str.c_str());
}
};
#endif

// This is the actual Android example. You don't have to write any platform
// specific code, unless you want to use things not directly exposed.
// ('vibrate()' in this example; undefine 'USE_JNI' above to disable it)
int main(int argc, char *argv[])
{
  sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "");

  sf::Texture texture;
  if(!texture.loadFromFile("image.png"))
    return EXIT_FAILURE;

  sf::Sprite image(texture);
  image.setPosition(0, 0);
  image.setOrigin(texture.getSize().x/2, texture.getSize().y/2);

  sf::Music music;
  if(!music.openFromFile("canary.wav"))
    return EXIT_FAILURE;

  music.play();

  sf::View view = window.getDefaultView();

    bool focus = true;

  while (window.isOpen())
  {
    sf::Event event;

    while (window.pollEvent(event))
    {
      switch (event.type)
      {
        case sf::Event::Closed:
          window.close();
              break;
        case sf::Event::Resized:
          view.setSize(event.size.width, event.size.height);
              view.setCenter(event.size.width/2, event.size.height/2);
              window.setView(view);
              break;
        case sf::Event::TouchBegan:
          if (event.touch.finger == 0)
          {
            image.setPosition(event.touch.x, event.touch.y);
#if defined(USE_JNI)
            vibrate(sf::milliseconds(100));

              testCall("dkjfhjskdhfjskdhgkjdfhgjkdfhgdfjkgdfhkjgjkdfghkjldf");

#endif
          }
              break;
          case sf::Event::LostFocus:

              focus = false;                    //don't draw, if the window is not shown
              LOGI("LOST FOCUS!");

              break;

          case sf::Event::GainedFocus:

              focus = true;                    //draw if the window is shown
              LOGI("GAINED FOCUS!");
              break;


          case sf::Event::MouseEntered: // mouse entered event is called on activity resume

              LOGI("MOUSE ENTERED!");

              window.create(sf::VideoMode::getDesktopMode(), "");   //recreating the window circumvents a nasty sfml-bug where on activityResume the egl_surface is not valid anymore

              testCall("MouseDidEnter");


              break;
      }
    }

      if (focus)
      {
          window.clear(sf::Color::White);
          window.draw(image);
          window.display();
      }
  }

    return 0;
}
