with open('file3.txt', 'w') as f:
    for i in range(10):
        f.write('Hello World! This is a long line of text. ' * 50)  # Each line is 1000 characters