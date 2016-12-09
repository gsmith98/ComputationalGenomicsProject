#!/usr/bin/env python3
import sys
import subprocess

if __name__ == "__main__":
    alphabets = ["IVFYWLMC_AHT_ED_GP_KNQRS","D_P_G_A_T_V_L_R_Y","CILMV_FYW_AGPST_DEHKNQR","M_W_C_KRQE_DNASTPGH_VILFY","C_H_AG_FILMVWY_DEKNPQRST","G_P_IVFYW_ALMEQRK_NDHSTC","KREDQN_C_G_H_ILV_M_F_Y_W_P_STA","A_R_N_D_C_E_Q_G_H_I_L_K_M_F_P_S_T_W_Y_V","STQNGPAHRED_LIFVMYWCK"]
    #modes = ["", "--sensitive","--more-sensitive"]
    modes = ["--sensitive"]


    callHead = "./diamond-0.8.26/bin/diamond blastx -d nr -q reads.fna -f 6 qseqid sseqid evalue bitscore length gaps ppos"
    callTail = " |  tail -3 | ./parseDiamondOutput.py >> data.csv"
    for i in range(1):
        for mode in modes:
            for alphabet in alphabets:
                filename = "alpha_" + mode + "_" + alphabet + "_" + str(i) + ".m8"
                callFlags = " -o dataDir/" + filename + " " + mode + " --reduced-alphabet " + alphabet
                print(callHead + callFlags + callTail)

                subprocess.call("echo -n \\\"" + filename + "\\\" >> data.csv", shell=True) 
                subprocess.call(callHead + callFlags + callTail, shell=True)
