#!/usr/bin/env python3

import glob
import numpy
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from collections import Counter

#Generate Data

sen = {}
num = {}
unique_num = {}
avg_gaps = {}
avg_ppos = {}
avg_repeat = {}
max_evalue = {}

alphabets = ["IVFYWLMC_AHT_ED_GP_KNQRS","D_P_G_A_T_V_L_R_Y","CILMV_FYW_AGPST_DEHKNQR","M_W_C_KRQE_DNASTPGH_VILFY","STQNGPAHRED_LIFVMYWCK","C_H_AG_FILMVWY_DEKNPQRST","G_P_IVFYW_ALMEQRK_NDHSTC","KREDQN_C_G_H_ILV_M_F_Y_W_P_STA","A_R_N_D_C_E_Q_G_H_I_L_K_M_F_P_S_T_W_Y_V"]
for name in glob.glob('../dataDir/*'):
    if "_0" in name:
        if "__" in name:
            seq_length = []
            seq_name = []
            gaps = []
            ppos = []
            evalue = []
            bscore = []
            repeats = {}
            count_repeats = []
            with open(name) as f:
                for line in f:
                    data = line.split()
                    seq_length.append(int(data[4]))
                    seq_name.append(data[0])
                    evalue.append(float(data[2]))
                    gaps.append(float(data[5]))
                    ppos.append(float(data[6]))
            for item in alphabets:
                if item in name:
                    sen[item] = numpy.mean(seq_length)
                    num[item] = len(seq_length)
                    unique_num[item] = len(numpy.unique(seq_name))
                    avg_gaps[item] = numpy.mean(gaps)
                    avg_ppos[item] = numpy.mean(ppos)
                    max_evalue[item] = max(evalue)
                    count = 0
                    repeats = dict(Counter(seq_name))
                    for key in repeats:
                        count += int(repeats[key])
                    avg_repeat[item] = float(count / len(repeats))

#generate plots
output = []
output2 = []
output3 = []
output4 = []
output5 = []
output6 = []

for item in alphabets:
    output.append(float(sen[item]))
    output2.append(int(num[item]))
    output3.append(int(unique_num[item]))
    output4.append(float(avg_gaps[item]))
    output5.append(float(avg_ppos[item]))
    output6.append(float(avg_repeat[item]))
#plot 1
fig = plt.figure()
fig.suptitle('Average length of Alphabet', fontsize=14, fontweight='bold')
ax = fig.add_subplot(111)
fig.subplots_adjust(top=0.9)

ax.set_xlabel('Alphabet')
ax.set_ylabel('Aligned Sequence Lengths')

labels = ['Alpha 1', 'Alpha 2', 'Alpha 3', 'Alpha 4', 'Alpha 5', 'Alpha 6', 'Alpha 7', 'Alpha 8', 'Alpha 9']
x = [0,1,2,3,4,5,6,7,8]
ax.plot(output)

plt.xticks(x, labels)

# Save the figure
fig.savefig('../figs/fig.png')
plt.clf()

#plot 2
fig = plt.figure()
fig.suptitle('Number of Matches', fontsize=14, fontweight='bold')
ax = fig.add_subplot(111)
fig.subplots_adjust(top=0.9)

ax.set_xlabel('Alphabet')
ax.set_ylabel('Count')

labels = ['Alpha 1', 'Alpha 2', 'Alpha 3', 'Alpha 4', 'Alpha 5', 'Alpha 6', 'Alpha 7', 'Alpha 8', 'Alpha 9']
x = [0,1,2,3,4,5,6,7,8]
ax.plot(output2)

plt.xticks(x, labels)

# Save the figure
fig.savefig('../figs/fig2.png')
plt.clf()

#plot 3
fig = plt.figure()
fig.suptitle('Number of Sequences Matched', fontsize=14, fontweight='bold')
ax = fig.add_subplot(111)
fig.subplots_adjust(top=0.9)

ax.set_xlabel('Alphabet')
ax.set_ylabel('Count')

labels = ['Alpha 1', 'Alpha 2', 'Alpha 3', 'Alpha 4', 'Alpha 5', 'Alpha 6', 'Alpha 7', 'Alpha 8', 'Alpha 9']
x = [0,1,2,3,4,5,6,7,8]
ax.plot(output3)

plt.xticks(x, labels)

# Save the figure
fig.savefig('../figs/fig3.png')
plt.clf()

#plot 4
fig = plt.figure()
fig.suptitle('Number of Gaps', fontsize=14, fontweight='bold')
ax = fig.add_subplot(111)
fig.subplots_adjust(top=0.9)

ax.set_xlabel('Alphabet')
ax.set_ylabel('Count')

labels = ['Alpha 1', 'Alpha 2', 'Alpha 3', 'Alpha 4', 'Alpha 5', 'Alpha 6', 'Alpha 7', 'Alpha 8', 'Alpha 9']
x = [0,1,2,3,4,5,6,7,8]
ax.plot(output4)

plt.xticks(x, labels)

# Save the figure
fig.savefig('../figs/fig4.png')
plt.clf()

#plot 5
fig = plt.figure()
fig.suptitle('Percentage of Positive Scoring Matches', fontsize=14, fontweight='bold')
ax = fig.add_subplot(111)
fig.subplots_adjust(top=0.9)

ax.set_xlabel('Alphabet')
ax.set_ylabel('Count')

labels = ['Alpha 1', 'Alpha 2', 'Alpha 3', 'Alpha 4', 'Alpha 5', 'Alpha 6', 'Alpha 7', 'Alpha 8', 'Alpha 9']
x = [0,1,2,3,4,5,6,7,8]
ax.plot(output5)

plt.xticks(x, labels)

# Save the figure
fig.savefig('../figs/fig5.png')
plt.clf()

#plot 6
fig = plt.figure()
fig.suptitle('Average hits per sequence', fontsize=14, fontweight='bold')
ax = fig.add_subplot(111)
fig.subplots_adjust(top=0.9)

ax.set_xlabel('Alphabet')
ax.set_ylabel('Count')

labels = ['Alpha 1', 'Alpha 2', 'Alpha 3', 'Alpha 4', 'Alpha 5', 'Alpha 6', 'Alpha 7', 'Alpha 8', 'Alpha 9']
x = [0,1,2,3,4,5,6,7,8]
ax.plot(output6)

plt.xticks(x, labels)
# Save the figure
fig.savefig('../figs/fig6.png')
plt.clf()
