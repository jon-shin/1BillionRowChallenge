# 1BillionRowChallenge
This is a public copy of the 1 billion row challenge, originally created for CS447: High Performance Computing.

The 1 billion row challenge was designed by Gunnar Morling, which challenged programmers to create an application which can parse through a text file with 1 billion lines which contain the name of a weather station and the temperature at the station. The program has to find the min, max, and mean temperature for each station: https://github.com/gunnarmorling/1brc

Though the challenge was intended for Java, the implementation was done in C++.

My implementation can parse the 1 billion lines in around 2 minutes, which is under half the time of the baseline implementation, which is 4:49.