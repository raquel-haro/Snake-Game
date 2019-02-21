#makefile to compile ECE251 programs
# usage:
#  make t=<target name>
t=
name=$(t)
CC=gcc
CFLAGS=

# these lines are here for working on other operating systems
# If you are on a Mac or Linux, this will automatically
# set the correct OpenGL parameters for you
OS = $(shell uname)
ifeq ($(OS), Darwin)
	LIBS= -framework OpenGL -framework GLUT
	CCFLAGS+=-Wno-deprecated
else ifeq ($(OS), Linux)
	LIBS= -lglut -lGLU -lGL
else
	LIBS= glut32.lib -lopengl32 -lglu32
endif

$(name): $(name).c
	$(CC) $(CFLAGS) -o $(name) $(name).c $(LIBS)
