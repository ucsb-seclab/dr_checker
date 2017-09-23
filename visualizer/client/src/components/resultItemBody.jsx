
import React from 'react';
import PropTypes from 'prop-types';
import { CardContent } from 'material-ui/Card';
import Collapse from 'material-ui/transitions/Collapse';
import Divider from 'material-ui/Divider';
import Typography from 'material-ui/Typography';
import { withStyles } from 'material-ui/styles';
import ContextResult from './contextResult.jsx';

const styles = theme => ({
  byTitle: {
    color: theme.palette.primary[300],
  },
});

/**
 * This class represent the body of the resultItem.
 * It encapsulatesvarious subcomponents responsble to show for example
 * the highlighted source code, the summary of the analysis relative to this
 * this function, etc...
 */
class ResultItemBody extends React.PureComponent {
  render() {
    const classes = this.props.classes;
    return (
      <Collapse in={this.props.expanded} transitionDuration="auto" unmountOnExit>
        <Divider />
        <CardContent>
          <Typography type="body2" className={classes.byTitle}>
            By Context
          </Typography>
          {
            this.props.alreadyFetched ?
              (this.props.contexts.map((val, idx) => (
                <ContextResult
                  key={`Context ${idx}`}
                  contextTitle={`Context ${idx}`}
                  warnings={val}
                  alreadyFetched={this.props.alreadyFetched}
                />
              )))
              :
              (<h4>Loading...</h4>)
          }
          <Typography type="body2" className={classes.byTitle}>
            By Instruction
          </Typography>
          {
            this.props.alreadyFetched ?
              (Object.keys(this.props.instructions).map(key => (
                <ContextResult
                  key={key}
                  contextTitle={key}
                  warnings={this.props.instructions[key]}
                  alreadyFetched={this.props.alreadyFetched}
                />
              )))
              :
              (<h4>Loading...</h4>)
          }

        </CardContent>
      </Collapse>
    );
  }
}

ResultItemBody.propTypes = {
  // toggle card collapse
  expanded: PropTypes.bool.isRequired,
  // list of contexts
  contexts: PropTypes.arrayOf(PropTypes.object).isRequired,
  instructions: PropTypes.object.isRequired,
  alreadyFetched: PropTypes.bool.isRequired,
  classes: PropTypes.object.isRequired,
};

export default withStyles(styles)(ResultItemBody);
