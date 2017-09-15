
import React from 'react';
import PropTypes from 'prop-types';
import { CardContent } from 'material-ui/Card';
import Collapse from 'material-ui/transitions/Collapse';
import Divider from 'material-ui/Divider';
import Grid from 'material-ui/Grid';
import Typography from 'material-ui/Typography';
import { withStyles } from 'material-ui/styles';
import SectionTitle from './sectionTitle.jsx';
import SourcecodeSection from './sourcecodeSection.jsx';
import SummarySection from './summarySection.jsx';

const styles = theme => ({
  gridTableSummary: {
    borderLeft: `1px solid ${theme.palette.primary[500]}`,
  },
});

/**
 * This class represent the body of the resultItem.
 * It encapsulatesvarious subcomponents responsble to show for example
 * the highlighted source code, the summary of the analysis relative to this
 * this function, etc...
 */
function ResultItemBody(props) {
  const classes = props.classes;
  return (
    <Collapse in={props.expanded} transitionDuration="auto" unmountOnExit>
      <Divider />
      <CardContent>
        <Grid container>
          <Grid item xs={12}>
            <SectionTitle title="Summary" />
          </Grid>
        </Grid>
        <Grid container>
          <Grid item xs={2}>
            <Typography type="title">
                    Warnings
            </Typography>
          </Grid>
          <Grid item xs={10} className={classes.gridTableSummary}>
            <SummarySection />
          </Grid>
        </Grid>
        <Grid container>
          <Grid item xs={12}>
            <SectionTitle title="Source code" />
          </Grid>
        </Grid>
        <SourcecodeSection />
      </CardContent>
    </Collapse>
  );
}

ResultItemBody.propTypes = {
  expanded: PropTypes.bool.isRequired,
  classes: PropTypes.object.isRequired,
};

export default withStyles(styles)(ResultItemBody);
