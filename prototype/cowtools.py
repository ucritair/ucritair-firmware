def foldl(f, acc, xs):
    if len(xs) == 0:
        return acc;
    else:
        h, t = xs[0], xs[1:];
        return foldl(f, f(acc, h), t);

def foldr(f, acc, xs):
    if len(xs) == 0:
        return acc;
    else:
        h, t = xs[0], xs[1:];
        return f(head, foldr(f, acc, t));
