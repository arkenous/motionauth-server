PROGRAM = authserver

NVCC = nvcc
CXXFLAGS = -std=c++11 -O3 --compiler-options '-fPIC' -shared
JAVA_HOME = /usr/lib/jvm/java-8-jdk
INCLUDE = -Icpp/include -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux
C_TARGET = out/lib$(PROGRAM).so
C_SOURCES = $(wildcard cpp/src/*.cpp) $(wildcard cpp/src/*.cu)

JAVAC = javac
JAVAH = javah
JAVACFLAGS = -sourcepath java/src -d out
JAVAHFLAGS = -classpath out -d out/
JAVAC_TARGET_MAIN = out/Main.class
JAVAC_TARGET_SOCKETPROCESSOR = out/SocketProcessor.class
JAVAH_TARGET_MAIN = out/Main.h
JAVAH_TARGET_SOCKETPROCESSOR = out/SocketProcessor.h

all: $(C_TARGET) $(JAVAC_TARGET_MAIN) $(JAVAC_TARGET_SOCKETPROCESSOR) $(JAVAH_TARGET_MAIN) $(JAVAH_TARGET_SOCKETPROCESSOR)

$(C_TARGET): $(C_SOURCES)
	$(NVCC) $(CXXFLAGS) $^ $(INCLUDE) -o $@

$(JAVAC_TARGET_MAIN): java/src/Main.java
	$(JAVAC) $(JAVACFLAGS) $^

$(JAVAC_TARGET_SOCKETPROCESSOR): java/src/SocketProcessor.java
	$(JAVAC) $(JAVACFLAGS) $^

$(JAVAH_TARGET_MAIN): java/src/Main.java
	$(JAVAH) $(JAVAHFLAGS) Main

$(JAVAH_TARGET_SOCKETPROCESSOR): java/src/SocketProcessor.java
	$(JAVAH) $(JAVAHFLAGS) SocketProcessor

.PHONY: clean
clean:
	$(RM) $(C_TARGET) $(JAVAC_TARGET_MAIN) $(JAVAC_TARGET_SOCKETPROCESSOR) $(JAVAH_TARGET_MAIN) $(JAVAH_TARGET_SOCKETPROCESSOR)
