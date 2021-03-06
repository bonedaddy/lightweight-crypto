
include ../../options.mak

.PHONY: all clean check generate

CXXFLAGS += -g $(COMMON_CFLAGS) $(STDC_CXXFLAGS)

TARGET = genavr

OBJS = \
	algorithm_ascon.o \
	algorithm_cham.o \
	algorithm_gascon.o \
	algorithm_gift128.o \
	algorithm_gift128_fixsliced.o \
	algorithm_gift64.o \
	algorithm_gimli24.o \
	algorithm_grain128.o \
	algorithm_keccak.o \
	algorithm_knot.o \
	algorithm_photon256.o \
	algorithm_pyjamask.o \
	algorithm_saturnin.o \
	algorithm_simp.o \
	algorithm_skinny128.o \
        algorithm_sliscp_light.o \
	algorithm_sparkle.o \
	algorithm_speck64.o \
	algorithm_spongent.o \
	algorithm_spook.o \
	algorithm_subterranean.o \
	algorithm_tinyjambu.o \
	algorithm_wage.o \
	algorithm_xoodoo.o \
	code.o \
	code_out.o \
	interpret.o \
        main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)

check: all
	./$(TARGET) --test

code.o: code.h
code_out.o: code.h
algorithm_ascon.o: code.h gen.h
algorithm_cham.o: code.h gen.h
algorithm_gascon.o: code.h gen.h
algorithm_gift128.o: code.h gen.h
algorithm_gift128_fixsliced.o: code.h gen.h
algorithm_gift64.o: code.h gen.h
algorithm_gimli24.o: code.h gen.h
algorithm_grain128.o: code.h gen.h
algorithm_keccak.o: code.h gen.h
algorithm_knot.o: code.h gen.h
algorithm_photon256.o: code.h gen.h
algorithm_pyjamask.o: code.h gen.h
algorithm_saturnin.o: code.h gen.h
algorithm_simp.o: code.h gen.h
algorithm_skinny128.o: code.h gen.h
algorithm_sliscp_light.o: code.h gen.h
algorithm_sparkle.o: code.h gen.h
algorithm_speck64.o: code.h gen.h
algorithm_spongent.o: code.h gen.h
algorithm_spook.o: code.h gen.h
algorithm_subterranean.o: code.h gen.h
algorithm_tinyjambu.o: code.h gen.h
algorithm_wage.o: code.h gen.h
algorithm_xoodoo.o: code.h gen.h
interpret.o: code.h

generate: $(TARGET)
	./$(TARGET) ASCON >../combined/internal-ascon-avr.S
	./$(TARGET) CHAM >../combined/internal-cham-avr.S
	./$(TARGET) GASCON >../combined/internal-drysponge-avr.S
	./$(TARGET) GIFT-128b-fs-4 >../combined/internal-gift128b-tiny-avr.S
	./$(TARGET) GIFT-128b-fs-20 >../combined/internal-gift128b-small-avr.S
	./$(TARGET) GIFT-128b-fs-80 >../combined/internal-gift128b-full-avr.S
	./$(TARGET) GIFT-128n-fs-4 >../combined/internal-gift128n-tiny-avr.S
	./$(TARGET) GIFT-128n-fs-20 >../combined/internal-gift128n-small-avr.S
	./$(TARGET) GIFT-128n-fs-80 >../combined/internal-gift128n-full-avr.S
	./$(TARGET) GIFT-64 >../combined/internal-gift64-avr.S
	./$(TARGET) GIMLI-24 >../combined/internal-gimli24-avr.S
	./$(TARGET) Grain-128 >../combined/internal-grain128-avr.S
	./$(TARGET) Keccak >../combined/internal-keccak-avr.S
	./$(TARGET) KNOT-256 >../combined/internal-knot-256-avr.S
	./$(TARGET) KNOT-384 >../combined/internal-knot-384-avr.S
	./$(TARGET) KNOT-512 >../combined/internal-knot-512-avr.S
	./$(TARGET) PHOTON-256 >../combined/internal-photon256-avr.S
	./$(TARGET) Pyjamask >../combined/internal-pyjamask-avr.S
	./$(TARGET) Saturnin >../combined/internal-saturnin-avr.S
	./$(TARGET) SimP >../combined/internal-simp-avr.S
	./$(TARGET) SKINNY-128 >../combined/internal-skinny128-avr.S
	./$(TARGET) sLiSCP-light-256-SPIX >../combined/internal-sliscp-256-spix-avr.S
	./$(TARGET) sLiSCP-light-256-SpoC >../combined/internal-sliscp-256-spoc-avr.S
	./$(TARGET) sLiSCP-light-192 >../combined/internal-sliscp-192-avr.S
	./$(TARGET) sLiSCP-light-320 >../combined/internal-sliscp-320-avr.S
	./$(TARGET) SPARKLE >../combined/internal-sparkle-avr.S
	./$(TARGET) SPECK-64 >../combined/internal-speck64-avr.S
	./$(TARGET) Spongent-pi >../combined/internal-spongent-avr.S
	./$(TARGET) Spook >../combined/internal-spook-avr.S
	./$(TARGET) Subterranean >../combined/internal-subterranean-avr.S
	./$(TARGET) TinyJAMBU >../combined/internal-tinyjambu-avr.S
	./$(TARGET) WAGE >../combined/internal-wage-avr.S
	./$(TARGET) Xoodoo >../combined/internal-xoodoo-avr.S
