
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
import FullScreenSourceCode from './fullScreenSourcecodeDialog.jsx';

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
class ResultItemBody extends React.PureComponent {
  constructor() {
    super();
    this.state = {
      // data retrieved from the server representing the details of the single result
      data: [
        { id: 1, name: 'warning 1', color: 'red', lineNo: 3, info: 'bla bla bla' },
        { id: 2, name: 'warning 2', color: 'green', lineNo: 5, info: 'bla bla bla 2' },
        { id: 3, name: 'warning 3', color: 'yellow', lineNo: 7, info: 'bla bla bla 3' },
        { id: 4, name: 'warning 4', color: 'orange', lineNo: 10, info: 'bla bla bla 4' },
      ],
      // warnings selected by the users that needs to be highlighted
      selectedWarnings: [],
      // line numbers to highlight
      highlightedLines: [],
      // toggle fullscreen visualization for the sourcecode
      showFullScreen: false,
    };
  }

  /**
   * Extacts which warnings nbeeds to be hioghlighted and their correscponfding line
   */
  handleWarningsSelection = (selectedIds) => {
    const toHighlight = this.state.data
      .filter(v => selectedIds.indexOf(v.id) !== -1)
      .map(n => ({ color: n.color, lineNo: n.lineNo }));
    this.setState({
      selectedWarnings: selectedIds,
      highlightedLines: toHighlight,
    });
  }

  render() {
    const classes = this.props.classes;
    return (
      <Collapse in={this.props.expanded} transitionDuration="auto" unmountOnExit>
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
              <SummarySection
                data={this.state.data}
                selected={this.state.selectedWarnings}
                handleSelection={this.handleWarningsSelection}
              />
            </Grid>
          </Grid>
          <Grid container>
            <Grid item xs={12}>
              <SectionTitle title="Source code" />
            </Grid>
          </Grid>
          <FullScreenSourceCode title={this.props.filename}>
            <SourcecodeSection highlightedLines={this.state.highlightedLines} />
          </FullScreenSourceCode>
          <SourcecodeSection highlightedLines={this.state.highlightedLines} overflowHidden />
        </CardContent>
      </Collapse>
    );
  }
}

ResultItemBody.propTypes = {
  // toggle card collapse
  expanded: PropTypes.bool.isRequired,
  classes: PropTypes.object.isRequired,
  // name of the file related to the result
  filename: PropTypes.string.isRequired,
};

export default withStyles(styles)(ResultItemBody);
