import React from 'react';
import PropTypes from 'prop-types';
import Typography from 'material-ui/Typography';
import { withStyles } from 'material-ui/styles';

const styles = theme => ({
  title: {
    color: theme.palette.common.fullWhite,
    borderBottom: `1px solid ${theme.palette.secondary[300]}`,
    fontFamily: 'monospace',
    marginBottom: 10,
  },
});

/**
 * Simple class to set the style for the section title 
 */
function SectionTitle(props) {
  const classes = props.classes;

  return (
    <Typography
      type="subheading"
      align="center"
      className={classes.title}
    >
      {props.title.toUpperCase()}
    </Typography>
  );
}

SectionTitle.propTypes = {
  classes: PropTypes.object.isRequired,
  title: PropTypes.string.isRequired,
};

export default withStyles(styles)(SectionTitle);
