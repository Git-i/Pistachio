#!/usr/bin/env python3
import fileinput
with fileinput.FileInput("Pistachio/CMakeLists.txt", inplace=True, backup='.bak') as file:
    for line in file:
        print(line.replace("$ENV{Projectname}", "${CMAKE_PROJECT_NAME}"), end='')
with fileinput.FileInput("Sandbox/CMakeLists.txt", inplace=True, backup='.bak') as file:
    for line in file:
        print(line.replace("$ENV{Projectname}", "${CMAKE_PROJECT_NAME}"), end='')
with fileinput.FileInput("Editor/CMakeLists.txt", inplace=True, backup='.bak') as file:
    for line in file:
        print(line.replace("$ENV{Projectname}", "${CMAKE_PROJECT_NAME}"), end='')