with open("test1.txt") as f1:
    with open("test2.txt") as f2:
        i = 0
        while l1 := f1.readline():
            l2 = f2.readline()
            if l1 != l2:
                break
            i += 1
        print(i)
