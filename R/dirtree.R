dirtree <- function(
    type = "irv",
    ptype = "ones",
    candidates = 5
){
    if(type=="irv"){
        root <- dirtree.irv(candidates = candidates, ptype = ptype)
    }else{
        stop("Expected one of: 'irv'.")
    }
}