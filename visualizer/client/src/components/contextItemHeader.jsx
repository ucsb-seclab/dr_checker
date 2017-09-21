import React from 'react';
import PropTypes from 'prop-types';
import classnames from 'classnames';
import { CardActions } from 'material-ui/Card';
import Typography from 'material-ui/Typography';
import ExpandMoreIcon from 'material-ui-icons/ExpandMore';
import IconButton from 'material-ui/IconButton';
import { withStyles } from 'material-ui/styles';

const styles = theme => ({
  cardHeader: {
    borderBottom: `1px solid ${theme.palette.common.fullWhite}`,
  },
  cardTitle: {
    paddingLeft: 12,
  },
  expand: {
    transform: 'rotate(0deg)',
    transition: theme.transitions.create('transform', {
      duration: theme.transitions.duration.shortest,
    }),
  },
  expandOpen: {
    transform: 'rotate(180deg)',
  },
  flexGrow: {
    flex: '1 1 auto',
  },
});

/**
 * This class represent the header of the contextItem.
 * It shows the name of the function which the result refer to and implements
 * the button to expand/collapse the result.
 */
function ContextItemHeader(props) {
  const classes = props.classes;

  return (
    <CardActions disableActionSpacing className={classes.cardHeader}>
      <Typography type="subheading" className={classes.cardTitle}>
        {props.title}
      </Typography>
      <div className={classes.flexGrow} />
      <IconButton
        className={classnames(classes.expand, {
          [classes.expandOpen]: props.expanded,
        })}
        onClick={props.handleExpandClick}
        aria-expanded={props.expanded}
        aria-label="Show more"
      >
        <ExpandMoreIcon />
      </IconButton>
    </CardActions>
  );
}

ContextItemHeader.propTypes = {
  classes: PropTypes.object.isRequired,
  title: PropTypes.string.isRequired,
  expanded: PropTypes.bool.isRequired,
  handleExpandClick: PropTypes.func.isRequired,
};

export default withStyles(styles)(ContextItemHeader);
