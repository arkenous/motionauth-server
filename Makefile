PROGRAM = authserver

NVCC = nvcc
CXXFLAGS = -gencode=arch=compute_30,code=sm_30 -gencode=arch=compute_32,code=sm_32 -gencode=arch=compute_35,code=sm_35 -gencode=arch=compute_50,code=sm_50 -gencode=arch=compute_52,code=sm_52 -gencode=arch=compute_60,code=sm_60, -gencode=arch=compute_61,code=sm_61 -gencode=arch=compute_61,code=compute_61 -std=c++11 -O3 --compiler-options '-fPIC' -shared
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
