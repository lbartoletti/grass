#include <stdlib.h>

#include <grass/raster.h>
#include <grass/imagery.h>
#include <grass/glocale.h>

#include "files.h"
#include "parms.h"

int get_training_classes(struct parms *parms,
    struct files *files, struct SigSet *S)
{
    int fd;
    CELL *cell;
    CELL cat;
    struct Cell_stats cell_stats;
    CELL *list;
    int row, nrows, ncols;
    int i, n;
    long count;
    struct ClassSig *Sig;

    fd = files->train_fd;
    cell = files->train_cell;

    nrows = Rast_window_rows();
    ncols = Rast_window_cols();

    /* determine the non-zero categories in the map */
    I_SetSigTitle(S, Rast_get_cats_title(&files->training_labels));

    Rast_init_cell_stats(&cell_stats);
    G_message(_("Finding training classes..."));
    for (row = 0; row < nrows; row++) {
        G_percent(row, nrows, 2);
        Rast_get_c_row(fd, cell, row);
        Rast_update_cell_stats(cell, ncols, &cell_stats);
    }
    G_percent(nrows, nrows, 2);

    /* convert this to an array */
    Rast_rewind_cell_stats(&cell_stats);
    n = 0;
    while (Rast_next_cell_stat(&cat, &count, &cell_stats)) {
        if (count > 1) {
            Sig = I_NewClassSig(S);
            I_SetClassTitle(Sig, Rast_get_c_cat(&cat,
                    &files->training_labels));
            Sig->classnum = cat;
            /* initialize this class with maxsubclasses (by allocating them) */
            for (i = 0; i < parms->maxsubclasses; i++)
                I_NewSubSig(S, Sig);
            I_AllocClassData(S, Sig, count);
            n++;
        }
        else
            G_warning(_("Training class %d only has one cell - this class will be ignored"),
                cat);
    }

    if (n == 0) {
        G_fatal_error(_("Training map has no classes"));
    }

    list = (CELL *) G_calloc(n, sizeof(CELL));
    n = 0;
    Rast_rewind_cell_stats(&cell_stats);
    while (Rast_next_cell_stat(&cat, &count, &cell_stats))
        if (count > 1)
            list[n++] = cat;

    Rast_free_cell_stats(&cell_stats);

    files->ncats = n;
    files->training_cats = list;

    if (files->ncats == 1)
        G_message(_("1 class found"));
    else
        G_message(_("%d classes found"), files->ncats);

    return 0;
}
