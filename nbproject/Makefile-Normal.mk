#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-Normal.mk)" "nbproject/Makefile-local-Normal.mk"
include nbproject/Makefile-local-Normal.mk
endif
endif

# Environment
MKDIR=mkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=Normal
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/cambadge.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/cambadge.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=cambadge.c hardware.c interrupts.c MDD_File_System/FSIO.c MDD_File_System/SD-SPI.c globals.c display.c serial.c fileformats.c particle.c browser.c camera.c apptemplate.c codescan.c utils.c imagefx.c scope.c box_game.c breakout.c tetrapuzz.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/cambadge.o ${OBJECTDIR}/hardware.o ${OBJECTDIR}/interrupts.o ${OBJECTDIR}/MDD_File_System/FSIO.o ${OBJECTDIR}/MDD_File_System/SD-SPI.o ${OBJECTDIR}/globals.o ${OBJECTDIR}/display.o ${OBJECTDIR}/serial.o ${OBJECTDIR}/fileformats.o ${OBJECTDIR}/particle.o ${OBJECTDIR}/browser.o ${OBJECTDIR}/camera.o ${OBJECTDIR}/apptemplate.o ${OBJECTDIR}/codescan.o ${OBJECTDIR}/utils.o ${OBJECTDIR}/imagefx.o ${OBJECTDIR}/scope.o ${OBJECTDIR}/box_game.o ${OBJECTDIR}/breakout.o ${OBJECTDIR}/tetrapuzz.o
POSSIBLE_DEPFILES=${OBJECTDIR}/cambadge.o.d ${OBJECTDIR}/hardware.o.d ${OBJECTDIR}/interrupts.o.d ${OBJECTDIR}/MDD_File_System/FSIO.o.d ${OBJECTDIR}/MDD_File_System/SD-SPI.o.d ${OBJECTDIR}/globals.o.d ${OBJECTDIR}/display.o.d ${OBJECTDIR}/serial.o.d ${OBJECTDIR}/fileformats.o.d ${OBJECTDIR}/particle.o.d ${OBJECTDIR}/browser.o.d ${OBJECTDIR}/camera.o.d ${OBJECTDIR}/apptemplate.o.d ${OBJECTDIR}/codescan.o.d ${OBJECTDIR}/utils.o.d ${OBJECTDIR}/imagefx.o.d ${OBJECTDIR}/scope.o.d ${OBJECTDIR}/box_game.o.d ${OBJECTDIR}/breakout.o.d ${OBJECTDIR}/tetrapuzz.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/cambadge.o ${OBJECTDIR}/hardware.o ${OBJECTDIR}/interrupts.o ${OBJECTDIR}/MDD_File_System/FSIO.o ${OBJECTDIR}/MDD_File_System/SD-SPI.o ${OBJECTDIR}/globals.o ${OBJECTDIR}/display.o ${OBJECTDIR}/serial.o ${OBJECTDIR}/fileformats.o ${OBJECTDIR}/particle.o ${OBJECTDIR}/browser.o ${OBJECTDIR}/camera.o ${OBJECTDIR}/apptemplate.o ${OBJECTDIR}/codescan.o ${OBJECTDIR}/utils.o ${OBJECTDIR}/imagefx.o ${OBJECTDIR}/scope.o ${OBJECTDIR}/box_game.o ${OBJECTDIR}/breakout.o ${OBJECTDIR}/tetrapuzz.o

# Source Files
SOURCEFILES=cambadge.c hardware.c interrupts.c MDD_File_System/FSIO.c MDD_File_System/SD-SPI.c globals.c display.c serial.c fileformats.c particle.c browser.c camera.c apptemplate.c codescan.c utils.c imagefx.c scope.c box_game.c breakout.c tetrapuzz.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-Normal.mk dist/${CND_CONF}/${IMAGE_TYPE}/cambadge.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=32MX170F256D
MP_LINKER_FILE_OPTION=,--script="link_forboot.ld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/cambadge.o: cambadge.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/cambadge.o.d 
	@${RM} ${OBJECTDIR}/cambadge.o 
	@${FIXDEPS} "${OBJECTDIR}/cambadge.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/cambadge.o.d" -o ${OBJECTDIR}/cambadge.o cambadge.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/hardware.o: hardware.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/hardware.o.d 
	@${RM} ${OBJECTDIR}/hardware.o 
	@${FIXDEPS} "${OBJECTDIR}/hardware.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/hardware.o.d" -o ${OBJECTDIR}/hardware.o hardware.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/interrupts.o: interrupts.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/interrupts.o.d 
	@${RM} ${OBJECTDIR}/interrupts.o 
	@${FIXDEPS} "${OBJECTDIR}/interrupts.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/interrupts.o.d" -o ${OBJECTDIR}/interrupts.o interrupts.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/MDD_File_System/FSIO.o: MDD_File_System/FSIO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/MDD_File_System" 
	@${RM} ${OBJECTDIR}/MDD_File_System/FSIO.o.d 
	@${RM} ${OBJECTDIR}/MDD_File_System/FSIO.o 
	@${FIXDEPS} "${OBJECTDIR}/MDD_File_System/FSIO.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/MDD_File_System/FSIO.o.d" -o ${OBJECTDIR}/MDD_File_System/FSIO.o MDD_File_System/FSIO.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/MDD_File_System/SD-SPI.o: MDD_File_System/SD-SPI.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/MDD_File_System" 
	@${RM} ${OBJECTDIR}/MDD_File_System/SD-SPI.o.d 
	@${RM} ${OBJECTDIR}/MDD_File_System/SD-SPI.o 
	@${FIXDEPS} "${OBJECTDIR}/MDD_File_System/SD-SPI.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/MDD_File_System/SD-SPI.o.d" -o ${OBJECTDIR}/MDD_File_System/SD-SPI.o MDD_File_System/SD-SPI.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/globals.o: globals.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/globals.o.d 
	@${RM} ${OBJECTDIR}/globals.o 
	@${FIXDEPS} "${OBJECTDIR}/globals.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/globals.o.d" -o ${OBJECTDIR}/globals.o globals.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/display.o: display.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/display.o.d 
	@${RM} ${OBJECTDIR}/display.o 
	@${FIXDEPS} "${OBJECTDIR}/display.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/display.o.d" -o ${OBJECTDIR}/display.o display.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/serial.o: serial.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/serial.o.d 
	@${RM} ${OBJECTDIR}/serial.o 
	@${FIXDEPS} "${OBJECTDIR}/serial.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/serial.o.d" -o ${OBJECTDIR}/serial.o serial.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/fileformats.o: fileformats.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/fileformats.o.d 
	@${RM} ${OBJECTDIR}/fileformats.o 
	@${FIXDEPS} "${OBJECTDIR}/fileformats.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/fileformats.o.d" -o ${OBJECTDIR}/fileformats.o fileformats.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/particle.o: particle.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/particle.o.d 
	@${RM} ${OBJECTDIR}/particle.o 
	@${FIXDEPS} "${OBJECTDIR}/particle.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/particle.o.d" -o ${OBJECTDIR}/particle.o particle.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/browser.o: browser.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/browser.o.d 
	@${RM} ${OBJECTDIR}/browser.o 
	@${FIXDEPS} "${OBJECTDIR}/browser.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/browser.o.d" -o ${OBJECTDIR}/browser.o browser.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/camera.o: camera.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/camera.o.d 
	@${RM} ${OBJECTDIR}/camera.o 
	@${FIXDEPS} "${OBJECTDIR}/camera.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/camera.o.d" -o ${OBJECTDIR}/camera.o camera.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/apptemplate.o: apptemplate.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/apptemplate.o.d 
	@${RM} ${OBJECTDIR}/apptemplate.o 
	@${FIXDEPS} "${OBJECTDIR}/apptemplate.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/apptemplate.o.d" -o ${OBJECTDIR}/apptemplate.o apptemplate.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/codescan.o: codescan.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/codescan.o.d 
	@${RM} ${OBJECTDIR}/codescan.o 
	@${FIXDEPS} "${OBJECTDIR}/codescan.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/codescan.o.d" -o ${OBJECTDIR}/codescan.o codescan.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/utils.o: utils.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/utils.o.d 
	@${RM} ${OBJECTDIR}/utils.o 
	@${FIXDEPS} "${OBJECTDIR}/utils.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/utils.o.d" -o ${OBJECTDIR}/utils.o utils.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/imagefx.o: imagefx.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/imagefx.o.d 
	@${RM} ${OBJECTDIR}/imagefx.o 
	@${FIXDEPS} "${OBJECTDIR}/imagefx.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/imagefx.o.d" -o ${OBJECTDIR}/imagefx.o imagefx.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/scope.o: scope.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/scope.o.d 
	@${RM} ${OBJECTDIR}/scope.o 
	@${FIXDEPS} "${OBJECTDIR}/scope.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/scope.o.d" -o ${OBJECTDIR}/scope.o scope.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/box_game.o: box_game.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/box_game.o.d 
	@${RM} ${OBJECTDIR}/box_game.o 
	@${FIXDEPS} "${OBJECTDIR}/box_game.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/box_game.o.d" -o ${OBJECTDIR}/box_game.o box_game.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/breakout.o: breakout.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/breakout.o.d 
	@${RM} ${OBJECTDIR}/breakout.o 
	@${FIXDEPS} "${OBJECTDIR}/breakout.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/breakout.o.d" -o ${OBJECTDIR}/breakout.o breakout.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/tetrapuzz.o: tetrapuzz.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/tetrapuzz.o.d 
	@${RM} ${OBJECTDIR}/tetrapuzz.o 
	@${FIXDEPS} "${OBJECTDIR}/tetrapuzz.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -DSimulator=1  -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/tetrapuzz.o.d" -o ${OBJECTDIR}/tetrapuzz.o tetrapuzz.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
else
${OBJECTDIR}/cambadge.o: cambadge.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/cambadge.o.d 
	@${RM} ${OBJECTDIR}/cambadge.o 
	@${FIXDEPS} "${OBJECTDIR}/cambadge.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/cambadge.o.d" -o ${OBJECTDIR}/cambadge.o cambadge.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/hardware.o: hardware.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/hardware.o.d 
	@${RM} ${OBJECTDIR}/hardware.o 
	@${FIXDEPS} "${OBJECTDIR}/hardware.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/hardware.o.d" -o ${OBJECTDIR}/hardware.o hardware.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/interrupts.o: interrupts.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/interrupts.o.d 
	@${RM} ${OBJECTDIR}/interrupts.o 
	@${FIXDEPS} "${OBJECTDIR}/interrupts.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/interrupts.o.d" -o ${OBJECTDIR}/interrupts.o interrupts.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/MDD_File_System/FSIO.o: MDD_File_System/FSIO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/MDD_File_System" 
	@${RM} ${OBJECTDIR}/MDD_File_System/FSIO.o.d 
	@${RM} ${OBJECTDIR}/MDD_File_System/FSIO.o 
	@${FIXDEPS} "${OBJECTDIR}/MDD_File_System/FSIO.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/MDD_File_System/FSIO.o.d" -o ${OBJECTDIR}/MDD_File_System/FSIO.o MDD_File_System/FSIO.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/MDD_File_System/SD-SPI.o: MDD_File_System/SD-SPI.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/MDD_File_System" 
	@${RM} ${OBJECTDIR}/MDD_File_System/SD-SPI.o.d 
	@${RM} ${OBJECTDIR}/MDD_File_System/SD-SPI.o 
	@${FIXDEPS} "${OBJECTDIR}/MDD_File_System/SD-SPI.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/MDD_File_System/SD-SPI.o.d" -o ${OBJECTDIR}/MDD_File_System/SD-SPI.o MDD_File_System/SD-SPI.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/globals.o: globals.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/globals.o.d 
	@${RM} ${OBJECTDIR}/globals.o 
	@${FIXDEPS} "${OBJECTDIR}/globals.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/globals.o.d" -o ${OBJECTDIR}/globals.o globals.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/display.o: display.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/display.o.d 
	@${RM} ${OBJECTDIR}/display.o 
	@${FIXDEPS} "${OBJECTDIR}/display.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/display.o.d" -o ${OBJECTDIR}/display.o display.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/serial.o: serial.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/serial.o.d 
	@${RM} ${OBJECTDIR}/serial.o 
	@${FIXDEPS} "${OBJECTDIR}/serial.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/serial.o.d" -o ${OBJECTDIR}/serial.o serial.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/fileformats.o: fileformats.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/fileformats.o.d 
	@${RM} ${OBJECTDIR}/fileformats.o 
	@${FIXDEPS} "${OBJECTDIR}/fileformats.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/fileformats.o.d" -o ${OBJECTDIR}/fileformats.o fileformats.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/particle.o: particle.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/particle.o.d 
	@${RM} ${OBJECTDIR}/particle.o 
	@${FIXDEPS} "${OBJECTDIR}/particle.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/particle.o.d" -o ${OBJECTDIR}/particle.o particle.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/browser.o: browser.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/browser.o.d 
	@${RM} ${OBJECTDIR}/browser.o 
	@${FIXDEPS} "${OBJECTDIR}/browser.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/browser.o.d" -o ${OBJECTDIR}/browser.o browser.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/camera.o: camera.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/camera.o.d 
	@${RM} ${OBJECTDIR}/camera.o 
	@${FIXDEPS} "${OBJECTDIR}/camera.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/camera.o.d" -o ${OBJECTDIR}/camera.o camera.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/apptemplate.o: apptemplate.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/apptemplate.o.d 
	@${RM} ${OBJECTDIR}/apptemplate.o 
	@${FIXDEPS} "${OBJECTDIR}/apptemplate.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/apptemplate.o.d" -o ${OBJECTDIR}/apptemplate.o apptemplate.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/codescan.o: codescan.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/codescan.o.d 
	@${RM} ${OBJECTDIR}/codescan.o 
	@${FIXDEPS} "${OBJECTDIR}/codescan.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/codescan.o.d" -o ${OBJECTDIR}/codescan.o codescan.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/utils.o: utils.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/utils.o.d 
	@${RM} ${OBJECTDIR}/utils.o 
	@${FIXDEPS} "${OBJECTDIR}/utils.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/utils.o.d" -o ${OBJECTDIR}/utils.o utils.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/imagefx.o: imagefx.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/imagefx.o.d 
	@${RM} ${OBJECTDIR}/imagefx.o 
	@${FIXDEPS} "${OBJECTDIR}/imagefx.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/imagefx.o.d" -o ${OBJECTDIR}/imagefx.o imagefx.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/scope.o: scope.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/scope.o.d 
	@${RM} ${OBJECTDIR}/scope.o 
	@${FIXDEPS} "${OBJECTDIR}/scope.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/scope.o.d" -o ${OBJECTDIR}/scope.o scope.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/box_game.o: box_game.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/box_game.o.d 
	@${RM} ${OBJECTDIR}/box_game.o 
	@${FIXDEPS} "${OBJECTDIR}/box_game.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/box_game.o.d" -o ${OBJECTDIR}/box_game.o box_game.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/breakout.o: breakout.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/breakout.o.d 
	@${RM} ${OBJECTDIR}/breakout.o 
	@${FIXDEPS} "${OBJECTDIR}/breakout.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/breakout.o.d" -o ${OBJECTDIR}/breakout.o breakout.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
${OBJECTDIR}/tetrapuzz.o: tetrapuzz.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/tetrapuzz.o.d 
	@${RM} ${OBJECTDIR}/tetrapuzz.o 
	@${FIXDEPS} "${OBJECTDIR}/tetrapuzz.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -g -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -O1 -MMD -MF "${OBJECTDIR}/tetrapuzz.o.d" -o ${OBJECTDIR}/tetrapuzz.o tetrapuzz.c    -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -fno-aggressive-loop-optimizations
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/cambadge.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    link_forboot.ld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE) -g -mdebugger -DSimulator=1 -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/cambadge.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)      -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,-D=__DEBUG_D,--defsym=Simulator=1,--defsym=_min_heap_size=512,--defsym=_min_stack_size=512,--gc-sections,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/cambadge.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   link_forboot.ld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION)  -o dist/${CND_CONF}/${IMAGE_TYPE}/cambadge.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -DXPRJ_Normal=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=_min_heap_size=512,--defsym=_min_stack_size=512,--gc-sections,--no-code-in-dinit,--no-dinit-in-serial-mem,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml
	${MP_CC_DIR}/xc32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/cambadge.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Normal
	${RM} -r dist/Normal

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell "${PATH_TO_IDE_BIN}"mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
