cmake_minimum_required(VERSION 3.8.0)

add_google_test(TickableTest  Pixie  Concepts/TickableTest.cpp)
add_google_test(SceneTest     Pixie  Core/SceneTest.cpp)
add_google_test(CoreTest      Pixie  Core/CoreTest.cpp)