CXXFLAGS+=-Wall -g

# Put here path to qbe-root
QBEROOT=qbe

BIN_DIR=./bin

SOURCES=$(wildcard *.cpp)
TARGET=$(addprefix $(BIN_DIR)/, $(basename $(notdir $(SOURCES))))

all: directories build_qbe $(TARGET)

$(TARGET):
	$(CXX) $(CXXFLAGS) $(wildcard $(notdir $@).cpp) $(QBEROOT)/obj/libqbe.a -o $@

directories:
	mkdir -p $(BIN_DIR)

build_qbe:
	$(MAKE) -C $(QBEROOT)

clean:
	rm -fr ./bin
	$(MAKE) -C $(QBEROOT) clean
