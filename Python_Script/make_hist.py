#Create images
import glob
import matplotlib.pyplot as plt

counter = 0

for name in glob.glob('C:/Users/Jack/Desktop/Data/Graph/*'):
    with open(name) as f:
        for line in f:
            data = line.split()
            seq_length.append(int(data[4]))
    fig = plt.figure()
    fig.suptitle('Histogram of Sequence Lengths', fontsize=14, fontweight='bold')
    ax = fig.add_subplot(111)
    fig.subplots_adjust(top=0.9)

    ax.set_xlabel('Aligned Sequence Lengths')
    ax.set_ylabel('Count')

    ax.hist(seq_length, bins=range(min(seq_length), max(seq_length) + 1, 1))

    # Save the figure
    fig.savefig('C:/Users/Jack/Desktop/Data/fig' + str(counter) + '.png')
    counter += 1
    plt.clf()
