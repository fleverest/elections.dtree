dirtree <- function(
    type = "irv",
    candidates = 5,
    ptype = "constant",
    scale = 1
){
    if(type=="irv"){
        root <- dirtree.irv(candidates = candidates, ptype = ptype)
    }else{
        stop("Expected one of: 'irv'.")
    }
}